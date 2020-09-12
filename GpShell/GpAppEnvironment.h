#pragma once

#include "GpDisplayDriverTickStatus.h"
#include "GpVOSEventQueue.h"
#include "HostSuspendCallID.h"

#include <stdint.h>

namespace PortabilityLayer
{
	union HostSuspendCallArgument;
	class HostFontHandler;
	class HostVOSEventQueue;
}

struct IGpDisplayDriver;
struct IGpAudioDriver;
struct IGpInputDriver;
struct IGpFiber;

class GpAppEnvironment
{
public:
	GpAppEnvironment();
	~GpAppEnvironment();

	void Init();

	GpDisplayDriverTickStatus_t Tick(IGpFiber *vosFiber);
	void Render();
	bool AdjustRequestedResolution(uint32_t &physicalWidth, uint32_t &physicalHeight, uint32_t &virtualWidth, uint32_t &virtualheight, float &pixelScaleX, float &pixelScaleY);

	void SetDisplayDriver(IGpDisplayDriver *displayDriver);
	void SetAudioDriver(IGpAudioDriver *audioDriver);
	void SetInputDrivers(IGpInputDriver *const* inputDrivers, size_t numDrivers);
	void SetFontHandler(PortabilityLayer::HostFontHandler *fontHandler);
	void SetVOSEventQueue(GpVOSEventQueue *eventQueue);

private:
	enum ApplicationState
	{
		ApplicationState_NotStarted,
		ApplicationState_WaitingForEvents,
		ApplicationState_Running,
		ApplicationState_Terminated,
		ApplicationState_SystemCall,
		ApplicationState_TimedSuspend,
	};

	static void StaticAppThreadFunc(void *context);
	void AppThreadFunc();
	void InitializeApplicationState();
	void SynchronizeState();

	static void StaticSuspendHookFunc(void *context, PortabilityLayer::HostSuspendCallID callID, const PortabilityLayer::HostSuspendCallArgument *args, PortabilityLayer::HostSuspendCallArgument *returnValue);
	void DispatchSystemCall(PortabilityLayer::HostSuspendCallID callID, const PortabilityLayer::HostSuspendCallArgument *args, PortabilityLayer::HostSuspendCallArgument *returnValue);

	ApplicationState m_applicationState;
	IGpDisplayDriver *m_displayDriver;
	IGpAudioDriver *m_audioDriver;
	IGpInputDriver *const* m_inputDrivers;
	PortabilityLayer::HostFontHandler *m_fontHandler;
	GpVOSEventQueue *m_vosEventQueue;
	IGpFiber *m_applicationFiber;
	IGpFiber *m_vosFiber;

	uint32_t m_delaySuspendTicks;
	size_t m_numInputDrivers;

	PortabilityLayer::HostSuspendCallID m_suspendCallID;
	const PortabilityLayer::HostSuspendCallArgument *m_suspendArgs;
	PortabilityLayer::HostSuspendCallArgument *m_suspendReturnValue;
};
