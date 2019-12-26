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
	//PL_NotYetImplemented_TODO("Time");
	return 0;
}

void GpSystemServices_Win32::GetLocalDateTime(unsigned int &year, unsigned int &month, unsigned int &day, unsigned int &hour, unsigned int &minute, unsigned int &second) const
{
	SYSTEMTIME localTime;
	GetLocalTime(&localTime);

	year = localTime.wYear;
	month = localTime.wMonth;
	day = localTime.wDay;
	hour = localTime.wHour;
	minute = localTime.wMinute;
	second = localTime.wSecond;
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
