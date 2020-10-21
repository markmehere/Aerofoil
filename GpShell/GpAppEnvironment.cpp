#include "GpAppEnvironment.h"
#include "GpFiberStarter.h"
#include "GpAppInterface.h"
#include "GpDisplayDriverTickStatus.h"
#include "GpFontHandlerFactory.h"
#include "HostSuspendCallArgument.h"
#include "IGpDisplayDriver.h"
#include "IGpFiber.h"
#include "IGpInputDriver.h"

#include <assert.h>

GpAppEnvironment::GpAppEnvironment()
	: m_applicationState(ApplicationState_NotStarted)
	, m_displayDriver(nullptr)
	, m_audioDriver(nullptr)
	, m_inputDrivers(nullptr)
	, m_numInputDrivers(0)
	, m_fontHandler(nullptr)
	, m_vosEventQueue(nullptr)
	, m_systemServices(nullptr)
	, m_applicationFiber(nullptr)
	, m_vosFiber(nullptr)
	, m_suspendCallID(PortabilityLayer::HostSuspendCallID_Unknown)
	, m_suspendArgs(nullptr)
	, m_suspendReturnValue(nullptr)
{
}

GpAppEnvironment::~GpAppEnvironment()
{
	assert(m_applicationFiber == nullptr);
}

void GpAppEnvironment::Init()
{
	GpAppInterface_Get()->ApplicationInit();
}

GpDisplayDriverTickStatus_t GpAppEnvironment::Tick(IGpFiber *vosFiber)
{
	GpAppInterface_Get()->PL_IncrementTickCounter(1);

	m_vosFiber = vosFiber;

	if (m_applicationState == ApplicationState_WaitingForEvents)
		m_applicationState = ApplicationState_Running;

	for (;;)
	{
		switch (m_applicationState)
		{
		case ApplicationState_NotStarted:
			InitializeApplicationState();
			m_applicationFiber = GpFiberStarter::StartFiber(m_systemServices, GpAppEnvironment::StaticAppThreadFunc, this, vosFiber);
			m_applicationState = ApplicationState_Running;
			break;
		case ApplicationState_WaitingForEvents:
			return GpDisplayDriverTickStatuses::kOK;
		case ApplicationState_Running:
			SynchronizeState();
			m_vosFiber->YieldTo(m_applicationFiber);
			break;
		case ApplicationState_SystemCall:
			{
				PortabilityLayer::HostSuspendCallID callID = m_suspendCallID;
				const PortabilityLayer::HostSuspendCallArgument *args = m_suspendArgs;
				PortabilityLayer::HostSuspendCallArgument *returnValue = m_suspendReturnValue;

				DispatchSystemCall(callID, args, returnValue);
				assert(m_applicationState != ApplicationState_SystemCall);
			}
			break;
		case ApplicationState_TimedSuspend:
			if (m_delaySuspendTicks == 0)
				m_applicationState = ApplicationState_Running;
			else
			{
				m_delaySuspendTicks--;
				return GpDisplayDriverTickStatuses::kOK;
			}
			break;
		case ApplicationState_Synchronizing:
			if (m_delaySuspendTicks == 0)
				m_applicationState = ApplicationState_Running;
			else
			{
				m_delaySuspendTicks--;
				return GpDisplayDriverTickStatuses::kSynchronizing;
			}
			break;
		case ApplicationState_Terminated:
			m_applicationFiber->Destroy();
			m_applicationFiber = nullptr;
			return GpDisplayDriverTickStatuses::kApplicationTerminated;
		default:
			assert(false);
			break;
		};
	}
}

void GpAppEnvironment::Render()
{
	GpAppInterface_Get()->PL_Render(m_displayDriver);
}

bool GpAppEnvironment::AdjustRequestedResolution(uint32_t &physicalWidth, uint32_t &physicalHeight, uint32_t &virtualWidth, uint32_t &virtualheight, float &pixelScaleX, float &pixelScaleY)
{
	return GpAppInterface_Get()->PL_AdjustRequestedResolution(physicalWidth, physicalHeight, virtualWidth, virtualheight, pixelScaleX, pixelScaleY);
}

void GpAppEnvironment::SetDisplayDriver(IGpDisplayDriver *displayDriver)
{
	m_displayDriver = displayDriver;
}

void GpAppEnvironment::SetAudioDriver(IGpAudioDriver *audioDriver)
{
	m_audioDriver = audioDriver;
}

void GpAppEnvironment::SetInputDrivers(IGpInputDriver *const* inputDrivers, size_t numDrivers)
{
	m_inputDrivers = inputDrivers;
	m_numInputDrivers = numDrivers;
}

void GpAppEnvironment::SetFontHandler(IGpFontHandler *fontHandler)
{
	m_fontHandler = fontHandler;
}

void GpAppEnvironment::SetVOSEventQueue(GpVOSEventQueue *eventQueue)
{
	m_vosEventQueue = eventQueue;
}

void GpAppEnvironment::SetSystemServices(PortabilityLayer::HostSystemServices *systemServices)
{
	m_systemServices = systemServices;
}

void GpAppEnvironment::StaticAppThreadFunc(void *context)
{
	static_cast<GpAppEnvironment*>(context)->AppThreadFunc();
}

void GpAppEnvironment::AppThreadFunc()
{
	GpAppInterface_Get()->ApplicationMain();

	m_applicationState = ApplicationState_Terminated;
	m_applicationFiber->YieldToTerminal(m_vosFiber);
}

void GpAppEnvironment::InitializeApplicationState()
{
	GpAppInterface_Get()->PL_HostDisplayDriver_SetInstance(m_displayDriver);
	GpAppInterface_Get()->PL_HostAudioDriver_SetInstance(m_audioDriver);
	GpAppInterface_Get()->PL_HostInputDriver_SetInstances(m_inputDrivers, m_numInputDrivers);
	GpAppInterface_Get()->PL_InstallHostSuspendHook(GpAppEnvironment::StaticSuspendHookFunc, this);

	GpAppInterface_Get()->PL_HostFontHandler_SetInstance(m_fontHandler);
	GpAppInterface_Get()->PL_HostVOSEventQueue_SetInstance(m_vosEventQueue);
}

void GpAppEnvironment::SynchronizeState()
{
	const size_t numInputDrivers = m_numInputDrivers;
	for (size_t i = 0; i < numInputDrivers; i++)
		m_inputDrivers[i]->ProcessInput();
}

void GpAppEnvironment::StaticSuspendHookFunc(void *context, PortabilityLayer::HostSuspendCallID callID, const PortabilityLayer::HostSuspendCallArgument *args, PortabilityLayer::HostSuspendCallArgument *returnValue)
{
	GpAppEnvironment *appEnv = static_cast<GpAppEnvironment*>(context);

	appEnv->m_suspendCallID = callID;
	appEnv->m_suspendArgs = args;
	appEnv->m_suspendReturnValue = returnValue;
	appEnv->m_applicationState = ApplicationState_SystemCall;

	appEnv->m_applicationFiber->YieldTo(appEnv->m_vosFiber);
}

void GpAppEnvironment::DispatchSystemCall(PortabilityLayer::HostSuspendCallID callID, const PortabilityLayer::HostSuspendCallArgument *args, PortabilityLayer::HostSuspendCallArgument *returnValue)
{
	switch (callID)
	{
	case PortabilityLayer::HostSuspendCallID_Delay:
		m_applicationState = ApplicationState_TimedSuspend;
		m_delaySuspendTicks = args[0].m_uint;
		break;
	case PortabilityLayer::HostSuspendCallID_ForceSyncFrame:
		m_applicationState = ApplicationState_Synchronizing;
		m_delaySuspendTicks = 1;
		break;
	case PortabilityLayer::HostSuspendCallID_CallOnVOSThread:
		args[0].m_functionPtr(args[1].m_pointer);
		m_applicationState = ApplicationState_Running;
		break;
	default:
		assert(false);
	}
}
