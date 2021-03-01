#include "GpSystemServices_Android.h"

#include "GpMutex_Cpp11.h"
#include "GpThreadEvent_Cpp11.h"
#include "SDL.h"

#include <time.h>
#include <unistd.h>

struct GpSystemServices_Android_ThreadStartParams
{
	GpSystemServices_Android::ThreadFunc_t m_threadFunc;
	void *m_threadContext;
	IGpThreadEvent *m_threadStartEvent;
};

static int SDLCALL StaticStartThread(void *lpThreadParameter)
{
	const GpSystemServices_Android_ThreadStartParams *threadParams = static_cast<const GpSystemServices_Android_ThreadStartParams*>(lpThreadParameter);

	GpSystemServices_Android::ThreadFunc_t threadFunc = threadParams->m_threadFunc;
	void *threadContext = threadParams->m_threadContext;
	IGpThreadEvent *threadStartEvent = threadParams->m_threadStartEvent;

	threadStartEvent->Signal();

	return threadFunc(threadContext);
}

GpSystemServices_Android::GpSystemServices_Android()
	: m_textInputEnabled(false)
{
}

int64_t GpSystemServices_Android::GetTime() const
{
	time_t t = time(nullptr);
	return static_cast<int64_t>(t) - 2082844800;
}

void GpSystemServices_Android::GetLocalDateTime(unsigned int &year, unsigned int &month, unsigned int &day, unsigned int &hour, unsigned int &minute, unsigned int &second) const
{
	time_t t = time(nullptr);
	tm *tmObject = localtime(&t);
	year = static_cast<unsigned int>(tmObject->tm_year);
	month = static_cast<unsigned int>(tmObject->tm_mon + 1);
	hour = static_cast<unsigned int>(tmObject->tm_hour);
	minute = static_cast<unsigned int>(tmObject->tm_min);
	second = static_cast<unsigned int>(tmObject->tm_sec);
}

IGpMutex *GpSystemServices_Android::CreateMutex()
{
	return GpMutex_Cpp11_NonRecursive::Create();
}


void *GpSystemServices_Android::CreateThread(ThreadFunc_t threadFunc, void *context)
{
	IGpThreadEvent *evt = CreateThreadEvent(true, false);
	if (!evt)
		return nullptr;

	GpSystemServices_Android_ThreadStartParams startParams;
	startParams.m_threadContext = context;
	startParams.m_threadFunc = threadFunc;
	startParams.m_threadStartEvent = evt;

	SDL_Thread *thread = SDL_CreateThread(StaticStartThread, "WorkerThread", &startParams);
	if (thread == nullptr)
	{
		evt->Destroy();
		return nullptr;
	}

	evt->Wait();
	evt->Destroy();

	return thread;
}

IGpMutex *GpSystemServices_Android::CreateRecursiveMutex()
{
	return GpMutex_Cpp11_Recursive::Create();
}

IGpThreadEvent *GpSystemServices_Android::CreateThreadEvent(bool autoReset, bool startSignaled)
{
	return GpThreadEvent_Cpp11::Create(autoReset, startSignaled);
}

uint64_t GpSystemServices_Android::GetFreeMemoryCosmetic() const
{
	long pages = sysconf(_SC_AVPHYS_PAGES);
	long pageSize = sysconf(_SC_PAGE_SIZE);
	return pages * pageSize;
}

void GpSystemServices_Android::Beep() const
{
}

bool GpSystemServices_Android::IsTouchscreen() const
{
	return true;
}

bool GpSystemServices_Android::IsUsingMouseAsTouch() const
{
	return false;
}

bool GpSystemServices_Android::IsTextInputObstructive() const
{
	return true;
}

bool GpSystemServices_Android::IsFullscreenPreferred() const
{
	return true;
}

unsigned int GpSystemServices_Android::GetCPUCount() const
{
	return SDL_GetCPUCount();
}

void GpSystemServices_Android::SetTextInputEnabled(bool isEnabled)
{
	m_textInputEnabled = isEnabled;
}

bool GpSystemServices_Android::IsTextInputEnabled() const
{
	return m_textInputEnabled;
}

bool GpSystemServices_Android::AreFontResourcesSeekable() const
{
	return false;
}

IGpClipboardContents *GpSystemServices_Android::GetClipboardContents() const
{
	return nullptr;
}

void GpSystemServices_Android::SetClipboardContents(IGpClipboardContents *contents)
{
}

GpSystemServices_Android *GpSystemServices_Android::GetInstance()
{
	return &ms_instance;
}

GpSystemServices_Android GpSystemServices_Android::ms_instance;
