#include "HostSystemServices.h"

namespace PortabilityLayer
{
	void HostSystemServices::SetInstance(HostSystemServices *instance)
	{
		ms_instance = instance;
	}

	HostSystemServices *HostSystemServices::GetInstance()
	{
		return ms_instance;
	}

	HostSystemServices *HostSystemServices::ms_instance;
}
