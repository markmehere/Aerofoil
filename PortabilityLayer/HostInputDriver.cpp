#include "HostInputDriver.h"

namespace PortabilityLayer
{
	size_t HostInputDriver::NumInstances()
	{
		return ms_numInstances;
	}

	IGpInputDriver *HostInputDriver::GetInstance(size_t index)
	{
		return ms_instances[index];
	}

	void HostInputDriver::SetInstances(IGpInputDriver *const* instances, size_t numInstances)
	{
		ms_instances = instances;
		ms_numInstances = numInstances;
	}

	IGpInputDriver *const* HostInputDriver::ms_instances;
	size_t HostInputDriver::ms_numInstances;
}
