#include "GpFiber_Win32.h"
#include <new>

GpFiber_Win32::GpFiber_Win32(LPVOID fiber)
	: m_fiber(fiber)
{
}

void GpFiber_Win32::YieldTo(IGpFiber *toFiber)
{
	SwitchToFiber(static_cast<GpFiber_Win32*>(toFiber)->m_fiber);
}

void GpFiber_Win32::Destroy()
{
	this->~GpFiber_Win32();
	free(this);
}

GpFiber_Win32::~GpFiber_Win32()
{
	DeleteFiber(m_fiber);
}

IGpFiber *GpFiber_Win32::Create(LPVOID fiber)
{
	void *storage = malloc(sizeof(GpFiber_Win32));
	if (!storage)
		return nullptr;

	return new (storage) GpFiber_Win32(fiber);
}
