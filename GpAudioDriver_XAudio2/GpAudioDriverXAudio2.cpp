#include "GpAudioDriverXAudio2.h"

#include "IGpLogDriver.h"
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
	IGpLogDriver *logger = properties.m_logger;

	IXAudio2 *xa = nullptr;
	IXAudio2MasteringVoice *mv = nullptr;

	const unsigned int realSampleRate = (properties.m_sampleRate + 50) / XAUDIO2_QUANTUM_DENOMINATOR * XAUDIO2_QUANTUM_DENOMINATOR;

	if (logger)
	{
		logger->Printf(IGpLogDriver::Category_Information, "XAudio2 Driver starting");
		logger->Printf(IGpLogDriver::Category_Information, "Real sample rate: %u", realSampleRate);
	}

	HRESULT result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (result != S_OK)
	{
		if (logger)
			logger->Printf(IGpLogDriver::Category_Error, "CoInitializeEx failed with code %lx", result);

		CoUninitialize();
		return nullptr;
	}

	UINT flags = 0;
	if (properties.m_debug)
	{
		if (logger)
			logger->Printf(IGpLogDriver::Category_Information, "Starting XAudio in debug mode");

		flags |= XAUDIO2_DEBUG_ENGINE;
	}

	result = XAudio2Create(&xa, flags, XAUDIO2_DEFAULT_PROCESSOR);
	if (FAILED(result))
	{
		if (logger)
			logger->Printf(IGpLogDriver::Category_Error, "XAudio2Create failed with code %lx", result);

		CoUninitialize();
		return nullptr;
	}

	result = xa->CreateMasteringVoice(&mv, 2, realSampleRate, 0, nullptr, nullptr, AudioCategory_GameEffects);
	if (FAILED(result))
	{
		if (logger)
			logger->Printf(IGpLogDriver::Category_Error, "CreateMasteringVoice failed with code %lx", result);

		CoUninitialize();
		xa->Release();
		return nullptr;
	}

	if (logger)
		logger->Printf(IGpLogDriver::Category_Information, "XAudio2 started OK", result);

	return new GpAudioDriverXAudio2(properties, realSampleRate, xa, mv);
}

IGpAudioChannel *GpAudioDriverXAudio2::CreateChannel()
{
	return GpAudioChannelXAudio2::Create(this);
}

void GpAudioDriverXAudio2::SetMasterVolume(uint32_t vol, uint32_t maxVolume)
{
	m_mv->SetVolume(static_cast<float>(vol) / static_cast<float>(maxVolume));
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
