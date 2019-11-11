#include "GpEvent.h"
#include "GpWindows.h"

#include <assert.h>

GpEvent::~GpEvent()
{
	CloseHandle(static_cast<HANDLE>(m_PrivateData));
}

void GpEvent::Wait()
{
	WaitForSingleObject(static_cast<HANDLE>(m_PrivateData), INFINITE);
}

void GpEvent::WaitMSec(unsigned int msec)
{
	assert(msec < MAXDWORD);
	WaitForSingleObject(static_cast<HANDLE>(m_PrivateData), static_cast<DWORD>(msec));
}

void GpEvent::Signal()
{
	SetEvent(static_cast<HANDLE>(m_PrivateData));
}

void GpEvent::Reset()
{
	ResetEvent(static_cast<HANDLE>(m_PrivateData));
}

void GpEvent::Destroy()
{
	delete this;
}

GpEvent *GpEvent::Create(bool autoReset, bool startSignalled)
{
	HANDLE handle = CreateEventA(nullptr, autoReset ? FALSE : TRUE, startSignalled ? TRUE : FALSE, nullptr);
	if (!handle)
		return nullptr;

	return new GpEvent(handle);
}

GpEvent::GpEvent(void *privateData)
	: m_PrivateData(privateData)
{
}
