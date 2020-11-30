#pragma once
#ifndef __PL_HOST_API_CALL_ID_H__
#define __PL_HOST_API_CALL_ID_H__

namespace PortabilityLayer
{
	enum HostSuspendCallID
	{
		HostSuspendCallID_Unknown,

		HostSuspendCallID_Delay,
		HostSuspendCallID_CallOnVOSThread,
		HostSuspendCallID_ForceSyncFrame,
	};
}

#endif
