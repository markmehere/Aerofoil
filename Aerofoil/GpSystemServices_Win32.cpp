#include "GpSystemServices_Win32.h"
#include "GpMutex_Win32.h"
#include "GpThreadEvent_Win32.h"

#include <assert.h>

#pragma push_macro("CreateMutex")
#ifdef CreateMutex
#undef CreateMutex
#endif

struct GpSystemServices_Win32_ThreadStartParams
{
	GpSystemServices_Win32::ThreadFunc_t m_threadFunc;
	void *m_threadContext;
	PortabilityLayer::HostThreadEvent *m_threadStartEvent;
};

static DWORD WINAPI StaticStartThread(LPVOID lpThreadParameter)
{
	const GpSystemServices_Win32_ThreadStartParams *threadParams = static_cast<const GpSystemServices_Win32_ThreadStartParams*>(lpThreadParameter);

	GpSystemServices_Win32::ThreadFunc_t threadFunc = threadParams->m_threadFunc;
	void *threadContext = threadParams->m_threadContext;
	PortabilityLayer::HostThreadEvent *threadStartEvent = threadParams->m_threadStartEvent;

	threadStartEvent->Signal();

	return threadFunc(threadContext);
}

GpSystemServices_Win32::GpSystemServices_Win32()
	: m_isTouchscreenSimulation(false)
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

	return (currentTime64 - epochStart64) / 10000000;
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

PortabilityLayer::HostMutex *GpSystemServices_Win32::CreateRecursiveMutex()
{
	return GpMutex_Win32::Create();
}

PortabilityLayer::HostThreadEvent *GpSystemServices_Win32::CreateThreadEvent(bool autoReset, bool startSignaled)
{
	return GpThreadEvent_Win32::Create(autoReset, startSignaled);
}

void *GpSystemServices_Win32::CreateThread(ThreadFunc_t threadFunc, void *context)
{
	PortabilityLayer::HostThreadEvent *evt = CreateThreadEvent(true, false);
	if (!evt)
		return nullptr;

	GpSystemServices_Win32_ThreadStartParams startParams;
	startParams.m_threadContext = context;
	startParams.m_threadFunc = threadFunc;
	startParams.m_threadStartEvent = evt;

	HANDLE threadHdl = ::CreateThread(nullptr, 0, StaticStartThread, &startParams, 0, nullptr);
	if (threadHdl == nullptr)
	{
		evt->Destroy();
		return nullptr;
	}

	evt->Wait();
	evt->Destroy();

	return threadHdl;
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

void GpSystemServices_Win32::Beep() const
{
	MessageBeep(MB_OK);
}

bool GpSystemServices_Win32::IsTouchscreen() const
{
	return m_isTouchscreenSimulation;
}

bool GpSystemServices_Win32::IsUsingMouseAsTouch() const
{
	return m_isTouchscreenSimulation;
}

bool GpSystemServices_Win32::IsTextInputObstructive() const
{
	return false;
}

unsigned int GpSystemServices_Win32::GetCPUCount() const
{
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	return sysInfo.dwNumberOfProcessors;
}

void GpSystemServices_Win32::SetTouchscreenSimulation(bool isTouchscreenSimulation)
{
	m_isTouchscreenSimulation = isTouchscreenSimulation;
}

GpSystemServices_Win32 *GpSystemServices_Win32::GetInstance()
{
	return &ms_instance;
}

GpSystemServices_Win32 GpSystemServices_Win32::ms_instance;


#pragma pop_macro("CreateMutex")
