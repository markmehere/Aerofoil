#include "HostFileSystem.h"

namespace PortabilityLayer
{
	HostFileSystem *HostFileSystem::GetInstance()
	{
		return ms_instance;
	}

	void HostFileSystem::SetInstance(HostFileSystem *instance)
	{
		ms_instance = instance;
	}

	HostFileSystem *HostFileSystem::ms_instance;
}
