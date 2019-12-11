#include "GpAudioDriverXAudio2.h"

#include "GpAudioChannelXAudio2.h"

#include <xaudio2.h>

void GpAudioDriverXAudio2::Shutdown()
{
	delete this;
}

const GpAudioDriverProperties &GpAudioDriverXAudio2::GetProperties() const
{
	return m_properties;
}

IXAudio2 *GpAudioDriverXAudio2::GetXA2() const
{
	return m_xa2;
}

IXAudio2MasteringVoice *GpAudioDriverXAudio2::GetMasteringVoice() const
{
	return m_mv;
}

unsigned int GpAudioDriverXAudio2::GetRealSampleRate() const
{
	return m_realSampleRate;
}

GpAudioDriverXAudio2 *GpAudioDriverXAudio2::Create(const GpAudioDriverProperties &properties)
{
	IXAudio2 *xa = nullptr;
	IXAudio2MasteringVoice *mv = nullptr;

	const unsigned int realSampleRate = (properties.m_sampleRate + 50) / XAUDIO2_QUANTUM_DENOMINATOR * XAUDIO2_QUANTUM_DENOMINATOR;

	if (CoInitializeEx(nullptr, COINIT_MULTITHREADED) != S_OK)
	{
		CoUninitialize();
		return nullptr;
	}

	UINT flags = 0;
	if (properties.m_debug)
		flags |= XAUDIO2_DEBUG_ENGINE;

	if (FAILED(XAudio2Create(&xa, flags, XAUDIO2_DEFAULT_PROCESSOR)))
	{
		CoUninitialize();
		return nullptr;
	}

	if (FAILED(xa->CreateMasteringVoice(&mv, 2, realSampleRate, 0, nullptr, nullptr, AudioCategory_GameEffects)))
	{
		CoUninitialize();
		xa->Release();
		return nullptr;
	}

	return new GpAudioDriverXAudio2(properties, realSampleRate, xa, mv);
}

IGpAudioChannel *GpAudioDriverXAudio2::CreateChannel()
{
	return GpAudioChannelXAudio2::Create(this);
}

GpAudioDriverXAudio2::GpAudioDriverXAudio2(const GpAudioDriverProperties &properties, unsigned int realSampleRate, IXAudio2* xa2, IXAudio2MasteringVoice *mv)
	: m_properties(properties)
	, m_realSampleRate(realSampleRate)
	, m_xa2(xa2)
	, m_mv(mv)
{
}

GpAudioDriverXAudio2::~GpAudioDriverXAudio2()
{
	m_mv->DestroyVoice();
	m_xa2->Release();

	CoUninitialize();
}
