#include "GpThreadEvent_Win32.h"
#include "IGpAllocator.h"

#include <stdlib.h>
#include <new>

void GpThreadEvent_Win32::Wait()
{
	WaitForSingleObject(m_event, INFINITE);
}

bool GpThreadEvent_Win32::WaitTimed(uint32_t msec)
{
	return WaitForSingleObject(m_event, static_cast<DWORD>(msec)) == WAIT_OBJECT_0;
}

void GpThreadEvent_Win32::Signal()
{
	SetEvent(m_event);
}

void GpThreadEvent_Win32::Destroy()
{
	IGpAllocator *alloc = m_alloc;
	this->~GpThreadEvent_Win32();
	alloc->Release(this);
}

GpThreadEvent_Win32 *GpThreadEvent_Win32::Create(IGpAllocator *alloc, bool autoReset, bool startSignaled)
{
	HANDLE handle = CreateEventA(nullptr, autoReset ? FALSE : TRUE, startSignaled ? TRUE : FALSE, nullptr);
	if (handle == nullptr)
		return nullptr;

	void *storage = alloc->Alloc(sizeof(GpThreadEvent_Win32));
	if (!storage)
	{
		CloseHandle(handle);
		return nullptr;
	}

	return new (storage) GpThreadEvent_Win32(alloc, handle);
}

GpThreadEvent_Win32::GpThreadEvent_Win32(IGpAllocator *alloc, const HANDLE &handle)
	: m_event(handle)
	, m_alloc(alloc)
{
}

GpThreadEvent_Win32::~GpThreadEvent_Win32()
{
	CloseHandle(m_event);
}
