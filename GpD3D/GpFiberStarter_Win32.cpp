#include "GpFiberStarter.h"
#include "GpFiber_Win32.h"
#include "GpWindows.h"

#include <assert.h>

namespace GpFiberStarter_Win32
{
	struct FiberStartState
	{
		GpFiberStarter::ThreadFunc_t m_threadFunc;
		GpFiber *m_creatingFiber;
		void *m_context;
	};

	static VOID WINAPI FiberStartRoutine(LPVOID lpThreadParameter)
	{
		const FiberStartState *tss = static_cast<const FiberStartState*>(lpThreadParameter);

		GpFiberStarter::ThreadFunc_t threadFunc = tss->m_threadFunc;
		GpFiber *creatingFiber = tss->m_creatingFiber;
		void *context = tss->m_context;
		creatingFiber->YieldTo();

		threadFunc(context);

		assert(!"Fiber function exited");
	}
}

GpFiber *GpFiberStarter::StartFiber(ThreadFunc_t threadFunc, void *context, GpFiber *creatingFiber)
{
	ULONG_PTR lowLimit;
	ULONG_PTR highLimit;

	GetCurrentThreadStackLimits(&lowLimit, &highLimit);

	ULONG_PTR stackSize = highLimit - lowLimit;

	GpFiberStarter_Win32::FiberStartState startState;
	startState.m_context = context;
	startState.m_creatingFiber = creatingFiber;
	startState.m_threadFunc = threadFunc;

	void *fiber = CreateFiber(static_cast<SIZE_T>(stackSize), GpFiberStarter_Win32::FiberStartRoutine, &startState);
	if (!fiber)
		return nullptr;

	SwitchToFiber(fiber);

	return new GpFiber_Win32(fiber);
}
