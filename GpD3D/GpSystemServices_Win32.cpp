#include "GpSystemServices_Win32.h"
#include "GpMutex_Win32.h"
#include "GpThreadEvent_Win32.h"

#include <assert.h>

#pragma push_macro("CreateMutex")
#ifdef CreateMutex
#undef CreateMutex
#endif

GpSystemServices_Win32::GpSystemServices_Win32()
{
}

uint32_t GpSystemServices_Win32::GetTime() const
{
	// PL_NotYetImplemented
	return 0;
}

PortabilityLayer::HostMutex *GpSystemServices_Win32::CreateMutex()
{
	return GpMutex_Win32::Create();
}

PortabilityLayer::HostThreadEvent *GpSystemServices_Win32::CreateThreadEvent(bool autoReset, bool startSignaled)
{
	return GpThreadEvent_Win32::Create(autoReset, startSignaled);
}

GpSystemServices_Win32 *GpSystemServices_Win32::GetInstance()
{
	return &ms_instance;
}

GpSystemServices_Win32 GpSystemServices_Win32::ms_instance;


#pragma pop_macro("CreateMutex")
