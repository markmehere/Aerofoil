#pragma once
#ifndef __PL_HOST_API_HOOK_H__
#define __PL_HOST_API_HOOK_H__

#include "HostSuspendCallID.h"

namespace PortabilityLayer
{
	union HostSuspendCallArgument;

	typedef void(*HostSuspendHook_t)(void *context, HostSuspendCallID callID, const HostSuspendCallArgument *args, HostSuspendCallArgument *returnValue);

	void InstallHostSuspendHook(HostSuspendHook_t hook, void *context);
	void SuspendApplication(HostSuspendCallID callID, const HostSuspendCallArgument *args, HostSuspendCallArgument *returnValue);
}

#endif
