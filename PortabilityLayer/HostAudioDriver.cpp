#include "HostAudioDriver.h"

namespace PortabilityLayer
{
	IGpAudioDriver *HostAudioDriver::GetInstance()
	{
		return ms_instance;
	}

	void HostAudioDriver::SetInstance(IGpAudioDriver *instance)
	{
		ms_instance = instance;
	}

	IGpAudioDriver *HostAudioDriver::ms_instance;
}
