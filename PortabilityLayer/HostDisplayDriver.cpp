#include "HostDisplayDriver.h"

namespace PortabilityLayer
{
	HostDisplayDriver *HostDisplayDriver::GetInstance()
	{
		return ms_instance;
	}

	void HostDisplayDriver::SetInstance(HostDisplayDriver *instance)
	{
		ms_instance = instance;
	}

	HostDisplayDriver *HostDisplayDriver::ms_instance;
}
