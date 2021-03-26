#pragma once

#include "GpDisplayDriverTickStatus.h"
#include "GpVOSEventQueue.h"
#include "HostSuspendCallID.h"

#include <stdint.h>

namespace PortabilityLayer
{
	union HostSuspendCallArgument;
}

struct IGpDisplayDriver;
struct IGpAudioDriver;
struct IGpFiber;
struct IGpFontHandler;
struct IGpInputDriver;
struct IGpSystemServices;
struct IGpVOSEventQueue;

class GpAppEnvironment
{
public:
	GpAppEnvironment();
	~GpAppEnvironment();

	void Init();
	void Run();

	void Render();
	bool AdjustRequestedResolution(uint32_t &physicalWidth, uint32_t &physicalHeight, uint32_t &virtualWidth, uint32_t &virtualheight, float &pixelScaleX, float &pixelScaleY);

	void SetDisplayDriver(IGpDisplayDriver *displayDriver);
	void SetAudioDriver(IGpAudioDriver *audioDriver);
	void SetInputDrivers(IGpInputDriver *const* inputDrivers, size_t numDrivers);
	void SetFontHandler(IGpFontHandler *fontHandler);
	void SetVOSEventQueue(GpVOSEventQueue *eventQueue);
	void SetSystemServices(IGpSystemServices *systemServices);

private:
	static void StaticAppThreadFunc(void *context);
	void InitializeApplicationState();
	void SynchronizeState();

	IGpDisplayDriver *m_displayDriver;
	IGpAudioDriver *m_audioDriver;
	IGpInputDriver *const* m_inputDrivers;
	IGpFontHandler *m_fontHandler;
	GpVOSEventQueue *m_vosEventQueue;
	IGpSystemServices *m_systemServices;

	uint32_t m_delaySuspendTicks;
	size_t m_numInputDrivers;

	PortabilityLayer::HostSuspendCallID m_suspendCallID;
	const PortabilityLayer::HostSuspendCallArgument *m_suspendArgs;
	PortabilityLayer::HostSuspendCallArgument *m_suspendReturnValue;
};
