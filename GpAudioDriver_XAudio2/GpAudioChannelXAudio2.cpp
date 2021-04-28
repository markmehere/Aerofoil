#include "GpAudioBufferXAudio2.h"
#include "GpAudioChannelXAudio2.h"
#include "GpAudioDriverXAudio2.h"
#include "IGpAllocator.h"
#include "IGpAudioChannelCallbacks.h"
#include "IGpLogDriver.h"

#include <stdlib.h>
#include <new>

GpAudioChannelXAudio2 *GpAudioChannelXAudio2::Create(IGpAllocator *alloc, GpAudioDriverXAudio2 *driver)
{
	IGpLogDriver *logger = driver->GetProperties().m_logger;

	void *storage = alloc->Realloc(nullptr, sizeof(GpAudioChannelXAudio2));
	if (!storage)
	{
		if (!logger)
			logger->Printf(IGpLogDriver::Category_Error, "GpAudioChannelXAudio2::Create failed, alloc failed");

		return nullptr;
	}

	GpAudioChannelXAudio2 *channel = new (storage) GpAudioChannelXAudio2(alloc, driver);
	if (!channel->Init())
	{
		if (!logger)
			logger->Printf(IGpLogDriver::Category_Error, "GpAudioChannelXAudio2::Init failed");

		channel->Destroy();
		return nullptr;
	}

	return channel;
}

bool GpAudioChannelXAudio2::Init()
{
	IGpLogDriver *logger = m_driver->GetProperties().m_logger;

	const unsigned int sampleRate = m_driver->GetRealSampleRate();
	IXAudio2 *const xa2 = m_driver->GetXA2();

	WAVEFORMATEX format;
	ZeroMemory(&format, sizeof(format));

	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = 1;
	format.nSamplesPerSec = sampleRate;
	format.wBitsPerSample = 8;
	format.nBlockAlign = format.nChannels * format.wBitsPerSample / 8;
	format.nAvgBytesPerSec = format.nBlockAlign * format.nSamplesPerSec;

	XAUDIO2_SEND_DESCRIPTOR sendsList[1];
	sendsList[0].Flags = 0;
	sendsList[0].pOutputVoice = m_driver->GetMasteringVoice();

	XAUDIO2_VOICE_SENDS sends;
	sends.pSends = sendsList;
	sends.SendCount = sizeof(sendsList) / sizeof(sendsList[0]);

	XAUDIO2_VOICE_DETAILS dets;
	m_driver->GetMasteringVoice()->GetVoiceDetails(&dets);

	HRESULT hr = xa2->CreateSourceVoice(&m_sourceVoice, &format, XAUDIO2_VOICE_NOPITCH | XAUDIO2_VOICE_NOSRC, 1.0f, &m_xAudioCallbacks, nullptr, nullptr);
	if (hr != S_OK)
	{
		if (!logger)
			logger->Printf(IGpLogDriver::Category_Error, "CreateSourceVoice failed with code %lx", hr);

		return false;
	}

	return true;
}

void GpAudioChannelXAudio2::SetAudioChannelContext(IGpAudioChannelCallbacks *callbacks)
{
	m_contextCallbacks = callbacks;
}

bool GpAudioChannelXAudio2::PostBuffer(IGpAudioBuffer *buffer)
{
	GpAudioBufferXAudio2 *xa2Buffer = static_cast<GpAudioBufferXAudio2*>(buffer);
	xa2Buffer->AddRef();

	HRESULT result = m_sourceVoice->SubmitSourceBuffer(xa2Buffer->GetXA2Buffer(), nullptr);
	if (result != S_OK)
	{
		xa2Buffer->Release();
		return false;
	}

	if (m_voiceState == VoiceState_Idle)
	{
		m_voiceState = VoiceState_Active;
		m_sourceVoice->Start(0, 0);
	}

	return true;
}

void GpAudioChannelXAudio2::Stop()
{
	// Set voice state BEFORE calling FlushSourceBuffers so state is idle before any callbacks trigger
	m_sourceVoice->Stop(0, 0);
	m_voiceState = VoiceState_Idle;

	m_sourceVoice->FlushSourceBuffers();
}

void GpAudioChannelXAudio2::Destroy()
{
	IGpAllocator *alloc = m_alloc;
	this->~GpAudioChannelXAudio2();
	alloc->Realloc(this, 0);
}

void GpAudioChannelXAudio2::OnBufferEnd()
{
	if (m_contextCallbacks)
		m_contextCallbacks->NotifyBufferFinished();
}

GpAudioChannelXAudio2::GpAudioChannelXAudio2(IGpAllocator *alloc, GpAudioDriverXAudio2 *driver)
	: m_driver(driver)
	, m_xAudioCallbacks(this)
	, m_sourceVoice(nullptr)
	, m_contextCallbacks(nullptr)
	, m_voiceState(VoiceState_Idle)
	, m_alloc(alloc)
{
}

GpAudioChannelXAudio2::~GpAudioChannelXAudio2()
{
	if (m_sourceVoice)
		m_sourceVoice->DestroyVoice();
}
