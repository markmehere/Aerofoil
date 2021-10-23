#include "HostSuspendHook.h"
#include "HostSuspendCallArgument.h"

#include "DisplayDeviceManager.h"

#include "PLDrivers.h"
#include "IGpDisplayDriver.h"
#include "IGpInputDriver.h"


namespace PortabilityLayer
{
	void RenderFrames(unsigned int ticks)
	{
		PLDrivers::GetDisplayDriver()->ServeTicks(ticks);
		DisplayDeviceManager::GetInstance()->IncrementTickCount(ticks);

		const size_t numInputDrivers = PLDrivers::GetNumInputDrivers();
		for (size_t i = 0; i < numInputDrivers; i++)
			PLDrivers::GetInputDriver(i)->ProcessInput();
	}
}
