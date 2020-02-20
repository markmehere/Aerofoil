#pragma once
#ifndef __PL_MOVIES_H__
#define __PL_MOVIES_H__

#include "PLApplication.h"
#include "VirtualDirectory.h"

namespace PortabilityLayer
{
	class ResourceArchive;
}

struct DrawSurface;
class AnimationPackage;

struct AnimationPlayer
{
	AnimationPackage *m_animPackage;
	Rect m_renderRect;
	Rect m_constrainRect;
	DrawSurface *m_surface;
	bool m_playing;

	uint32_t m_timer;
	uint16_t m_frameIndex;
	bool m_isFrameCurrent;

	AnimationPlayer *m_next;
	AnimationPlayer *m_prev;

	AnimationPlayer();

	void SetPackage(AnimationPackage *animPackage);
	void Restart();
};

class AnimationPackage final
{
public:
	static AnimationPackage *Create();
	void Destroy();

	bool Load(PortabilityLayer::VirtualDirectory_t virtualDir, const PLPasStr &path);

	const THandle<BitmapImage> &GetFrame(size_t index) const;
	size_t NumFrames() const;
	uint32_t GetFrameRateNumerator() const;
	uint32_t GetFrameRateDenominator() const;

private:
	explicit AnimationPackage();
	~AnimationPackage();

	THandle<BitmapImage> *m_images;
	PortabilityLayer::ResourceArchive *m_resArchive;
	size_t m_numImages;

	uint32_t m_frameRateNumerator;
	uint32_t m_frameRateDenominator;
};

struct AnimationPlayerRef
{
	AnimationPlayer *m_player;
};

class AnimationManager final
{
public:
	void RegisterPlayer(AnimationPlayer *player);
	void RemovePlayer(AnimationPlayer *player);

	void RefreshPlayer(AnimationPlayer *player);
	void TickPlayers(uint32_t numTicks);

	static AnimationManager *GetInstance();

private:
	AnimationManager();

	AnimationPlayer *m_firstPlayer;
	AnimationPlayer *m_lastPlayer;

	static AnimationManager ms_instance;
};

#endif
