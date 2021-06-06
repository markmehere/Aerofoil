#include "PLMovies.h"

#include "BitmapImage.h"
#include "FileManager.h"
#include "MemoryManager.h"
#include "PLQDraw.h"
#include "PLResources.h"
#include "QDManager.h"
#include "QDPixMap.h"
#include "ResourceManager.h"

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"


AnimationPlayer::AnimationPlayer()
	: m_animPackage(nullptr)
	, m_renderRect(Rect::Create(0, 0, 0, 0))
	, m_constrainRect(Rect::Create(0, 0, 0, 0))
	, m_surface(nullptr)
	, m_playing(false)
	, m_timer(0)
	, m_frameIndex(0)
	, m_isFrameCurrent(false)
	, m_next(nullptr)
	, m_prev(nullptr)
{
}

void AnimationPlayer::SetPackage(AnimationPackage *animPackage)
{
	m_animPackage = animPackage;
	Restart();
}

void AnimationPlayer::Restart()
{
	m_isFrameCurrent = (m_frameIndex == 0);

	m_playing = false;
	m_timer = 0;
	m_frameIndex = 0;
}

AnimationPackage *AnimationPackage::Create()
{
	void *storage = PortabilityLayer::MemoryManager::GetInstance()->Alloc(sizeof(AnimationPackage));
	if (!storage)
		return nullptr;

	return new (storage) AnimationPackage();
}

void AnimationPackage::Destroy()
{
	this->~AnimationPackage();
	PortabilityLayer::MemoryManager::GetInstance()->Release(this);
}

PLError_t AnimationPackage::Load(PortabilityLayer::VirtualDirectory_t dirID, const PLPasStr &name)
{
	m_compositeFile = PortabilityLayer::FileManager::GetInstance()->OpenCompositeFile(dirID, name);
	if (!m_compositeFile)
		return PLErrors::kFileNotFound;

	m_resArchive = PortabilityLayer::ResourceManager::GetInstance()->LoadResFile(m_compositeFile);
	if (!m_resArchive)
		return PLErrors::kResourceError;

	THandle<void> movieMetadataRes = m_resArchive->LoadResource('muvi', 0);
	if (!movieMetadataRes)
		return PLErrors::kResourceError;

	const void *movieMetadata = *movieMetadataRes;

	rapidjson::Document document;
	document.Parse(static_cast<const char*>(movieMetadata), movieMetadataRes.MMBlock()->m_size);

	movieMetadataRes.Dispose();

	if (document.HasParseError() || !document.IsObject())
		return PLErrors::kResourceError;

	if (!document.HasMember("frameRateNumerator") || !document.HasMember("frameRateDenominator"))
		return PLErrors::kResourceError;

	const rapidjson::Value &frameRateNumeratorJSON = document["frameRateNumerator"];
	const rapidjson::Value &frameRateDenominatorJSON = document["frameRateDenominator"];

	if (!frameRateNumeratorJSON.IsInt() && !frameRateDenominatorJSON.IsInt())
		return PLErrors::kResourceError;

	const int frameRateNumerator = frameRateNumeratorJSON.GetInt();
	const int frameRateDenominator = frameRateDenominatorJSON.GetInt();

	if (frameRateNumerator < 1 || frameRateDenominator < 1)
		return PLErrors::kResourceError;

	if (frameRateDenominator > INT_MAX / 60 || frameRateDenominator * 60 < frameRateNumerator)
		return PLErrors::kResourceError;	// We only support up to 60fps

	m_frameRateNumerator = frameRateNumerator;
	m_frameRateDenominator = frameRateDenominator;

	uint32_t numFrames = 0;
	for (;;)
	{
		if (numFrames + 1 > 0x7fff)
			return PLErrors::kResourceError;

		THandle<void> frameRes = m_resArchive->LoadResource('PICT', numFrames + 1);
		if (!frameRes)
			break;
		else
		{
			if (frameRes.MMBlock()->m_size < sizeof(BitmapImage))
				return PLErrors::kResourceError;

			numFrames++;
		}
	}

	if (numFrames == 0)
		return PLErrors::kResourceError;

	void *imageListStorage = PortabilityLayer::MemoryManager::GetInstance()->Alloc(sizeof(THandle<BitmapImage>) * numFrames);
	if (!imageListStorage)
		return PLErrors::kResourceError;

	m_images = static_cast<THandle<BitmapImage>*>(imageListStorage);

	for (uint32_t i = 0; i < numFrames; i++)
		new (m_images + i) THandle<BitmapImage>();

	for (uint32_t i = 0; i < numFrames; i++)
		m_images[i] = m_resArchive->LoadResource('PICT', i + 1).StaticCast<BitmapImage>();

	m_numImages = numFrames;

	return PLErrors::kNone;
}

const THandle<BitmapImage> &AnimationPackage::GetFrame(size_t index) const
{
	return m_images[index];
}

size_t AnimationPackage::NumFrames() const
{
	return m_numImages;
}

uint32_t AnimationPackage::GetFrameRateNumerator() const
{
	return m_frameRateNumerator;
}

uint32_t AnimationPackage::GetFrameRateDenominator() const
{
	return m_frameRateDenominator;
}


AnimationPackage::AnimationPackage()
	: m_images(nullptr)
	, m_resArchive(nullptr)
	, m_compositeFile(nullptr)
	, m_numImages(0)
{
}

AnimationPackage::~AnimationPackage()
{
	if (m_resArchive)
		m_resArchive->Destroy();

	if (m_compositeFile)
		m_compositeFile->Close();

	PortabilityLayer::MemoryManager::GetInstance()->Release(m_images);
}


void AnimationManager::RegisterPlayer(AnimationPlayer *player)
{
	assert(player->m_prev == nullptr || player->m_next == nullptr);

	if (!m_firstPlayer)
		m_firstPlayer = player;

	if (m_lastPlayer)
		m_lastPlayer->m_next = player;

	player->m_prev = m_lastPlayer;
	player->m_next = nullptr;

	m_lastPlayer = player;
}

void AnimationManager::RemovePlayer(AnimationPlayer *player)
{
	if (m_firstPlayer == player)
		m_firstPlayer = m_firstPlayer->m_next;

	if (m_lastPlayer == player)
		m_lastPlayer = m_lastPlayer->m_prev;

	if (player->m_prev)
		player->m_prev->m_next = player->m_next;
	if (player->m_next)
		player->m_next->m_prev = player->m_prev;

	player->m_prev = player->m_next = nullptr;
}

void AnimationManager::RefreshPlayer(AnimationPlayer *player)
{
	if (!player->m_playing || !player->m_animPackage || !player->m_surface)
		return;

	AnimationPackage *anim = player->m_animPackage;

	if (!player->m_isFrameCurrent)
	{
		player->m_isFrameCurrent = true;

		THandle<BitmapImage> img = anim->GetFrame(player->m_frameIndex);

		DrawSurface *surface = player->m_surface;

		if (player->m_renderRect == player->m_constrainRect)
			surface->DrawPicture(img, player->m_renderRect);
		else
		{
			DrawSurface *renderSurface = nullptr;
			if (PortabilityLayer::QDManager::GetInstance()->NewGWorld(&renderSurface, surface->m_port.GetPixelFormat(), player->m_renderRect) != PLErrors::kNone)
				return;

			renderSurface->DrawPicture(img, player->m_renderRect);

			CopyBits(*renderSurface->m_port.GetPixMap(), *surface->m_port.GetPixMap(), &player->m_constrainRect, &player->m_constrainRect, srcCopy);

			PortabilityLayer::QDManager::GetInstance()->DisposeGWorld(renderSurface);
		}
	}
}

void AnimationManager::TickPlayers(uint32_t numTicks)
{
	for (AnimationPlayer *player = m_firstPlayer; player; player = player->m_next)
	{
		AnimationPackage *anim = player->m_animPackage;

		uint32_t fptNumerator = anim->GetFrameRateNumerator();
		uint32_t fptDenominator = anim->GetFrameRateDenominator() * 60;

		for (uint32_t t = 0; t < numTicks; t++)
		{
			player->m_timer += fptNumerator;
			if (player->m_timer >= fptDenominator)
			{
				player->m_timer -= fptDenominator;
				player->m_frameIndex++;
				if (player->m_frameIndex == anim->NumFrames())
					player->m_frameIndex = 0;

				player->m_isFrameCurrent = false;
			}
		}
	}
}

AnimationManager *AnimationManager::GetInstance()
{
	return &ms_instance;
}

AnimationManager::AnimationManager()
	: m_firstPlayer(nullptr)
	, m_lastPlayer(nullptr)
{
}

AnimationManager AnimationManager::ms_instance;
