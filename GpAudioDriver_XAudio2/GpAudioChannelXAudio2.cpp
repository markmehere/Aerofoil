#include "GpAudioChannelXAudio2.h"
#include "GpAudioDriverXAudio2.h"
#include "IGpAudioChannelCallbacks.h"

#include <stdlib.h>
#include <new>

GpAudioChannelXAudio2 *GpAudioChannelXAudio2::Create(GpAudioDriverXAudio2 *driver)
{
	void *storage = malloc(sizeof(GpAudioChannelXAudio2));
	if (!storage)
		return nullptr;

	GpAudioChannelXAudio2 *channel = new (storage) GpAudioChannelXAudio2(driver);
	if (!channel->Init())
	{
		channel->Destroy();
		return nullptr;
	}

	return channel;
}

bool GpAudioChannelXAudio2::Init()
{
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
		return false;

	return true;
}

void GpAudioChannelXAudio2::SetAudioChannelContext(IGpAudioChannelCallbacks *callbacks)
{
	m_contextCallbacks = callbacks;
}

void GpAudioChannelXAudio2::PostBuffer(const void *buffer, size_t bufferSize)
{
	XAUDIO2_BUFFER xa2Buffer;
	xa2Buffer.Flags = 0;
	xa2Buffer.AudioBytes = static_cast<UINT32>(bufferSize);
	xa2Buffer.pAudioData = static_cast<const BYTE*>(buffer);
	xa2Buffer.PlayBegin = 0;
	xa2Buffer.PlayLength = 0;
	xa2Buffer.LoopBegin = 0;
	xa2Buffer.LoopLength = 0;
	xa2Buffer.LoopCount = 0;
	xa2Buffer.pContext = nullptr;

	m_sourceVoice->SubmitSourceBuffer(&xa2Buffer, nullptr);
	if (m_voiceState == VoiceState_Idle)
	{
		m_voiceState = VoiceState_Active;
		m_sourceVoice->Start(0, 0);
	}
}

void GpAudioChannelXAudio2::Destroy()
{
	this->~GpAudioChannelXAudio2();
	free(this);
}

void GpAudioChannelXAudio2::OnBufferEnd()
{
	if (m_contextCallbacks)
		m_contextCallbacks->NotifyBufferFinished();
}

GpAudioChannelXAudio2::GpAudioChannelXAudio2(GpAudioDriverXAudio2 *driver)
	: m_driver(driver)
	, m_xAudioCallbacks(this)
	, m_sourceVoice(nullptr)
	, m_contextCallbacks(nullptr)
	, m_voiceState(VoiceState_Idle)
{
}

GpAudioChannelXAudio2::~GpAudioChannelXAudio2()
{
	if (m_sourceVoice)
		m_sourceVoice->DestroyVoice();
}
