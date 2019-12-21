#include "GpAppEnvironment.h"
#include "GpFiberStarter.h"
#include "GpAppInterface.h"
#include "GpFontHandlerFactory.h"
#include "GpPLGlueAudioDriver.h"
#include "GpPLGlueDisplayDriver.h"
#include "GpFiber.h"
#include "HostSuspendCallArgument.h"

#include <assert.h>

GpAppEnvironment::GpAppEnvironment()
	: m_applicationState(ApplicationState_NotStarted)
	, m_displayDriver(nullptr)
	, m_audioDriver(nullptr)
	, m_fontHandler(nullptr)
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
}

void GpAppEnvironment::Tick(GpFiber *vosFiber)
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
			m_applicationFiber = GpFiberStarter::StartFiber(GpAppEnvironment::StaticAppThreadFunc, this, vosFiber);
			m_applicationState = ApplicationState_Running;
			break;
		case ApplicationState_WaitingForEvents:
			return;
		case ApplicationState_Running:
			m_applicationFiber->YieldTo();
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
				return;
			}
			break;
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

void GpAppEnvironment::SetDisplayDriver(IGpDisplayDriver *displayDriver)
{
	m_displayDriver = displayDriver;
}

void GpAppEnvironment::SetAudioDriver(IGpAudioDriver *audioDriver)
{
	m_audioDriver = audioDriver;
}

void GpAppEnvironment::SetFontHandler(PortabilityLayer::HostFontHandler *fontHandler)
{
	m_fontHandler = fontHandler;
}

void GpAppEnvironment::StaticAppThreadFunc(void *context)
{
	static_cast<GpAppEnvironment*>(context)->AppThreadFunc();
}

void GpAppEnvironment::AppThreadFunc()
{
	GpAppInterface_Get()->ApplicationMain();
}

void GpAppEnvironment::InitializeApplicationState()
{
	GpAppInterface_Get()->PL_HostDisplayDriver_SetInstance(GpPLGlueDisplayDriver::GetInstance());
	GpAppInterface_Get()->PL_HostAudioDriver_SetInstance(GpPLGlueAudioDriver::GetInstance());
	GpAppInterface_Get()->PL_InstallHostSuspendHook(GpAppEnvironment::StaticSuspendHookFunc, this);

	GpAppInterface_Get()->PL_HostFontHandler_SetInstance(m_fontHandler);
	GpAppInterface_Get()->PL_HostVOSEventQueue_SetInstance(&m_vosEventQueue);

	SynchronizeState();
}

void GpAppEnvironment::SynchronizeState()
{
	GpPLGlueDisplayDriver::GetInstance()->SetGpDisplayDriver(m_displayDriver);
	GpPLGlueAudioDriver::GetInstance()->SetGpAudioDriver(m_audioDriver);
}

void GpAppEnvironment::StaticSuspendHookFunc(void *context, PortabilityLayer::HostSuspendCallID callID, const PortabilityLayer::HostSuspendCallArgument *args, PortabilityLayer::HostSuspendCallArgument *returnValue)
{
	GpAppEnvironment *appEnv = static_cast<GpAppEnvironment*>(context);

	appEnv->m_suspendCallID = callID;
	appEnv->m_suspendArgs = args;
	appEnv->m_suspendReturnValue = returnValue;
	appEnv->m_applicationState = ApplicationState_SystemCall;

	appEnv->m_vosFiber->YieldTo();
}

void GpAppEnvironment::DispatchSystemCall(PortabilityLayer::HostSuspendCallID callID, const PortabilityLayer::HostSuspendCallArgument *args, PortabilityLayer::HostSuspendCallArgument *returnValue)
{
	switch (callID)
	{
	case PortabilityLayer::HostSuspendCallID_Delay:
		m_applicationState = ApplicationState_TimedSuspend;
		m_delaySuspendTicks = args[0].m_uint;
		break;
	default:
		assert(false);
	}
}
