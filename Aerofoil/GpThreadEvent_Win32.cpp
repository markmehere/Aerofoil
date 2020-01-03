#include "GpThreadEvent_Win32.h"

#include <stdlib.h>
#include <new>

void GpThreadEvent_Win32::Wait()
{
	WaitForSingleObject(m_event, INFINITE);
}

void GpThreadEvent_Win32::WaitTimed(uint32_t msec)
{
	WaitForSingleObject(m_event, static_cast<DWORD>(msec));
}

void GpThreadEvent_Win32::Signal()
{
	SetEvent(m_event);
}

void GpThreadEvent_Win32::Destroy()
{
	this->~GpThreadEvent_Win32();
	free(this);
}

GpThreadEvent_Win32 *GpThreadEvent_Win32::Create(bool autoReset, bool startSignaled)
{
	HANDLE handle = CreateEventA(nullptr, autoReset ? FALSE : TRUE, startSignaled ? TRUE : FALSE, nullptr);
	if (handle == nullptr)
		return nullptr;

	void *storage = malloc(sizeof(GpThreadEvent_Win32));
	if (!storage)
	{
		CloseHandle(handle);
		return nullptr;
	}

	return new (storage) GpThreadEvent_Win32(handle);
}

GpThreadEvent_Win32::GpThreadEvent_Win32(const HANDLE &handle)
	: m_event(handle)
{
}

GpThreadEvent_Win32::~GpThreadEvent_Win32()
{
	CloseHandle(m_event);
}
