#pragma once

#include "HostAudioDriver.h"

struct IGpAudioDriver;

class GpPLGlueAudioDriver final : public PortabilityLayer::HostAudioDriver
{
public:
	GpPLGlueAudioDriver();

	PortabilityLayer::HostAudioChannel *CreateChannel() override;
	void SetMasterVolume(uint32_t vol, uint32_t maxVolume) override;

	void SetGpAudioDriver(IGpAudioDriver *audioDriver);

	static GpPLGlueAudioDriver *GetInstance();

private:
	IGpAudioDriver *m_audioDriver;

	static GpPLGlueAudioDriver ms_instance;
};
