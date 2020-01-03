#pragma once

#include "HostAudioDriver.h"

struct IGpAudioDriver;

class GpPLGlueAudioDriver final : public PortabilityLayer::HostAudioDriver
{
public:
	GpPLGlueAudioDriver();

	PortabilityLayer::HostAudioChannel *CreateChannel() override;

	void SetGpAudioDriver(IGpAudioDriver *audioDriver);

	static GpPLGlueAudioDriver *GetInstance();

private:
	IGpAudioDriver *m_audioDriver;

	static GpPLGlueAudioDriver ms_instance;
};
