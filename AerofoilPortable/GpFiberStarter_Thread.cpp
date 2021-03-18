#include "GpFiberStarter.h"
#include "GpFiber_Thread.h"

#include "IGpSystemServices.h"
#include "IGpThreadEvent.h"

#include <assert.h>

namespace GpFiberStarter_Thread
{
	struct FiberStartState
	{
		GpFiberStarter::ThreadFunc_t m_threadFunc;
		IGpThreadEvent *m_creatingReturnEvent;
		IGpThreadEvent *m_creatingWakeEvent;
		void *m_context;
	};

	static int FiberStartRoutine(void *lpThreadParameter)
	{
		const FiberStartState *tss = static_cast<const FiberStartState*>(lpThreadParameter);

		GpFiberStarter::ThreadFunc_t threadFunc = tss->m_threadFunc;
		IGpThreadEvent *creatingReturnEvent = tss->m_creatingReturnEvent;
		IGpThreadEvent *wakeEvent = tss->m_creatingWakeEvent;
		void *context = tss->m_context;
		creatingReturnEvent->Signal();

		wakeEvent->Wait();

		threadFunc(context);

		return 0;
	}
}

IGpFiber *GpFiberStarter::StartFiber(IGpSystemServices *systemServices, ThreadFunc_t threadFunc, void *context, IGpFiber *creatingFiber)
{
	IGpThreadEvent *returnEvent = systemServices->CreateThreadEvent(true, false);
	if (!returnEvent)
		return nullptr;

	IGpThreadEvent *wakeEvent = systemServices->CreateThreadEvent(true, false);
	if (!wakeEvent)
	{
		returnEvent->Destroy();
		return nullptr;
	}

	GpFiberStarter_Thread::FiberStartState startState;
	startState.m_context = context;
	startState.m_creatingReturnEvent = returnEvent;
	startState.m_creatingWakeEvent = wakeEvent;
	startState.m_threadFunc = threadFunc;

	void *thread = systemServices->CreateThread(GpFiberStarter_Thread::FiberStartRoutine, &startState);
	if (!thread)
	{
		returnEvent->Destroy();
		wakeEvent->Destroy();
		return nullptr;
	}

	returnEvent->Wait();
	returnEvent->Destroy();

	return new GpFiber_Thread(thread, wakeEvent);
}
