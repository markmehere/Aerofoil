#include "GpMutex_Win32.h"

#include "GpWindows.h"

#include <stdlib.h>
#include <new>

void GpMutex_Win32::Destroy()
{
	this->~GpMutex_Win32();
	free(this);
}

void GpMutex_Win32::Lock()
{
	EnterCriticalSection(&m_critSection);
}

void GpMutex_Win32::Unlock()
{
	LeaveCriticalSection(&m_critSection);
}


GpMutex_Win32 *GpMutex_Win32::Create()
{
	void *storage = malloc(sizeof(GpMutex_Win32));
	if (!storage)
		return nullptr;

	return new (storage) GpMutex_Win32();
}

GpMutex_Win32::GpMutex_Win32()
{
	InitializeCriticalSection(&m_critSection);
}

GpMutex_Win32::~GpMutex_Win32()
{
	DeleteCriticalSection(&m_critSection);
}
