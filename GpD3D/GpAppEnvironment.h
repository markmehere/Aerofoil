#pragma once

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
class GpFiber;

class GpAppEnvironment
{
public:
	GpAppEnvironment();
	~GpAppEnvironment();

	void Init();

	void Tick(GpFiber *vosFiber);
	void Render();

	void SetDisplayDriver(IGpDisplayDriver *displayDriver);
	void SetAudioDriver(IGpAudioDriver *audioDriver);
	void SetFontHandler(PortabilityLayer::HostFontHandler *fontHandler);

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
	PortabilityLayer::HostFontHandler *m_fontHandler;
	GpVOSEventQueue m_vosEventQueue;
	GpFiber *m_applicationFiber;
	GpFiber *m_vosFiber;

	uint32_t m_delaySuspendTicks;

	PortabilityLayer::HostSuspendCallID m_suspendCallID;
	const PortabilityLayer::HostSuspendCallArgument *m_suspendArgs;
	PortabilityLayer::HostSuspendCallArgument *m_suspendReturnValue;
};
