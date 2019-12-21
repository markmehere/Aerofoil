#pragma once

struct GpVOSEvent;

namespace PortabilityLayer
{
	class HostVOSEventQueue
	{
	public:
		virtual const GpVOSEvent *GetNext() = 0;
		virtual void DischargeOne() = 0;

		static void SetInstance(HostVOSEventQueue *instance);
		static HostVOSEventQueue *GetInstance();

	private:
		static HostVOSEventQueue *ms_instance;
	};
}
