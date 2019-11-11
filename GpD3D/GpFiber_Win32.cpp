#include "GpFiber_Win32.h"

GpFiber_Win32::GpFiber_Win32(LPVOID fiber)
	: m_fiber(fiber)
{
}

void GpFiber_Win32::YieldTo()
{
	SwitchToFiber(m_fiber);
}

void GpFiber_Win32::Destroy()
{
	DeleteFiber(m_fiber);
	delete this;
}
