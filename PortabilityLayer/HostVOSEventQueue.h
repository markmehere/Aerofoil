#pragma once

#include "IGpVOSEventQueue.h"

namespace PortabilityLayer
{
	class HostVOSEventQueue : public IGpVOSEventQueue
	{
	public:
		static void SetInstance(HostVOSEventQueue *instance);
		static HostVOSEventQueue *GetInstance();

	private:
		static HostVOSEventQueue *ms_instance;
	};
}
