#include "HostDisplayDriver.h"

namespace PortabilityLayer
{
	IGpDisplayDriver *HostDisplayDriver::GetInstance()
	{
		return ms_instance;
	}

	void HostDisplayDriver::SetInstance(IGpDisplayDriver *instance)
	{
		ms_instance = instance;
	}

	IGpDisplayDriver *HostDisplayDriver::ms_instance;
}
