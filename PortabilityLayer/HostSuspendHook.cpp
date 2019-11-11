#include "HostSuspendHook.h"
#include "HostSuspendCallArgument.h"

namespace
{
	static PortabilityLayer::HostSuspendHook_t gs_suspendHook;
	static void *gs_suspendContext;
}

namespace PortabilityLayer
{
	void InstallHostSuspendHook(HostSuspendHook_t hook, void *context)
	{
		gs_suspendHook = hook;
		gs_suspendContext = context;
	}

	void SuspendApplication(HostSuspendCallID callID, const HostSuspendCallArgument *args, HostSuspendCallArgument *returnValue)
	{
		gs_suspendHook(gs_suspendContext, callID, args, returnValue);
	}
}
