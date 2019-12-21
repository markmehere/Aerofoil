#include "HostVOSEventQueue.h"

namespace PortabilityLayer
{
	void HostVOSEventQueue::SetInstance(HostVOSEventQueue *instance)
	{
		ms_instance = instance;
	}

	HostVOSEventQueue *HostVOSEventQueue::GetInstance()
	{
		return ms_instance;
	}

	HostVOSEventQueue *HostVOSEventQueue::ms_instance = nullptr;
}
