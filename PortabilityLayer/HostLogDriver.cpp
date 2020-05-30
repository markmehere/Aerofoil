#include "HostLogDriver.h"

namespace PortabilityLayer
{
	void HostLogDriver::SetInstance(IGpLogDriver *instance)
	{
		ms_instance = instance;
	}

	IGpLogDriver *HostLogDriver::GetInstance()
	{
		return ms_instance;
	}

	IGpLogDriver *HostLogDriver::ms_instance;
}
