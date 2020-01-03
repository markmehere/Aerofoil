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

int64_t GpSystemServices_Win32::GetTime() const
{
	SYSTEMTIME epochStart;
	epochStart.wYear = 1904;
	epochStart.wMonth = 1;
	epochStart.wDayOfWeek = 5;
	epochStart.wDay = 1;
	epochStart.wHour = 0;
	epochStart.wMinute = 0;
	epochStart.wSecond = 0;
	epochStart.wMilliseconds = 0;

	FILETIME epochStartFT;
	if (!SystemTimeToFileTime(&epochStart, &epochStartFT))
		return 0;

	FILETIME currentTime;
	GetSystemTimeAsFileTime(&currentTime);

	int64_t epochStart64 = (static_cast<int64_t>(epochStartFT.dwLowDateTime) & 0xffffffff) | (static_cast<int64_t>(epochStartFT.dwHighDateTime) << 32);
	int64_t currentTime64 = (static_cast<int64_t>(currentTime.dwLowDateTime) & 0xffffffff) | (static_cast<int64_t>(currentTime.dwHighDateTime) << 32);

	return currentTime64 - epochStart64;
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

uint64_t GpSystemServices_Win32::GetFreeMemoryCosmetic() const
{
	MEMORYSTATUSEX memStatus;
	memset(&memStatus, 0, sizeof(memStatus));

	memStatus.dwLength = sizeof(memStatus);

	if (!GlobalMemoryStatusEx(&memStatus))
		return 0;

	return memStatus.ullAvailPhys;
}

GpSystemServices_Win32 *GpSystemServices_Win32::GetInstance()
{
	return &ms_instance;
}

GpSystemServices_Win32 GpSystemServices_Win32::ms_instance;


#pragma pop_macro("CreateMutex")
