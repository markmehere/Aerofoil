#include "GpPLGlueAudioDriver.h"

#include "GpPLGlueAudioChannel.h"
#include "IGpAudioChannel.h"
#include "IGpAudioDriver.h"

GpPLGlueAudioDriver::GpPLGlueAudioDriver()
	: m_audioDriver(nullptr)
{
}

PortabilityLayer::HostAudioChannel *GpPLGlueAudioDriver::CreateChannel()
{
	IGpAudioChannel *channel = m_audioDriver->CreateChannel();
	if (!channel)
		return nullptr;

	PortabilityLayer::HostAudioChannel *glueChannel = GpPLGlueAudioChannel::Create(channel);
	if (!glueChannel)
	{
		channel->Destroy();
		return nullptr;
	}

	return glueChannel;
}

GpPLGlueAudioDriver *GpPLGlueAudioDriver::GetInstance()
{
	return &ms_instance;
}

void GpPLGlueAudioDriver::SetGpAudioDriver(IGpAudioDriver *audioDriver)
{
	m_audioDriver = audioDriver;
}

GpPLGlueAudioDriver GpPLGlueAudioDriver::ms_instance;
