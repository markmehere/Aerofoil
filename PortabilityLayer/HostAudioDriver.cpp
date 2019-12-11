#include "HostAudioDriver.h"

namespace PortabilityLayer
{
	HostAudioDriver *HostAudioDriver::GetInstance()
	{
		return ms_instance;
	}

	void HostAudioDriver::SetInstance(HostAudioDriver *instance)
	{
		ms_instance = instance;
	}

	HostAudioDriver *HostAudioDriver::ms_instance;
}
