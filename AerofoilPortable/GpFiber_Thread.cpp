#include "GpFiber_Thread.h"
#include "IGpThreadEvent.h"

GpFiber_Thread::GpFiber_Thread(void *thread, IGpThreadEvent *threadEvent)
	: m_event(threadEvent)
	, m_thread(thread)
{
}

GpFiber_Thread::~GpFiber_Thread()
{
	m_event->Destroy();
}

void GpFiber_Thread::YieldTo(IGpFiber *toFiber)
{
	static_cast<GpFiber_Thread*>(toFiber)->m_event->Signal();
	m_event->Wait();
}

void GpFiber_Thread::YieldToTerminal(IGpFiber *toFiber)
{
	static_cast<GpFiber_Thread*>(toFiber)->m_event->Signal();
}

void GpFiber_Thread::Destroy()
{
	delete this;
}
