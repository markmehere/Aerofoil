#include "GpMutex_Win32.h"

#include "IGpAllocator.h"
#include "GpWindows.h"

#include <stdlib.h>
#include <new>

void GpMutex_Win32::Destroy()
{
	IGpAllocator *alloc = m_alloc;
	this->~GpMutex_Win32();
	alloc->Release(this);
}

void GpMutex_Win32::Lock()
{
	EnterCriticalSection(&m_critSection);
}

void GpMutex_Win32::Unlock()
{
	LeaveCriticalSection(&m_critSection);
}


GpMutex_Win32 *GpMutex_Win32::Create(IGpAllocator *alloc)
{
	void *storage = alloc->Alloc(sizeof(GpMutex_Win32));
	if (!storage)
		return nullptr;

	return new (storage) GpMutex_Win32(alloc);
}

GpMutex_Win32::GpMutex_Win32(IGpAllocator *alloc)
	: m_alloc(alloc)
{
	InitializeCriticalSection(&m_critSection);
}

GpMutex_Win32::~GpMutex_Win32()
{
	DeleteCriticalSection(&m_critSection);
}
