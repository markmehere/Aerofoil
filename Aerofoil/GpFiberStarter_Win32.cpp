#include "GpFiberStarter.h"
#include "GpFiber_Win32.h"
#include "GpWindows.h"

#include <assert.h>

namespace GpFiberStarter_Win32
{
	struct FiberStartState
	{
		GpFiberStarter::ThreadFunc_t m_threadFunc;
		IGpFiber *m_creatingFiber;
		void *m_context;
	};

	static VOID WINAPI FiberStartRoutine(LPVOID lpThreadParameter)
	{
		const FiberStartState *tss = static_cast<const FiberStartState*>(lpThreadParameter);

		GpFiberStarter::ThreadFunc_t threadFunc = tss->m_threadFunc;
		IGpFiber *creatingFiber = tss->m_creatingFiber;
		void *context = tss->m_context;
		SwitchToFiber(static_cast<GpFiber_Win32*>(creatingFiber)->GetFiber());

		threadFunc(context);

		assert(!"Fiber function exited");
	}
}

IGpFiber *GpFiberStarter::StartFiber(IGpSystemServices *systemServices, ThreadFunc_t threadFunc, void *context, IGpFiber *creatingFiber)
{
	ULONG_PTR lowLimit;
	ULONG_PTR highLimit;

#if 0
	GetCurrentThreadStackLimits(&lowLimit, &highLimit);

	ULONG_PTR stackSize = highLimit - lowLimit;
#else
	ULONG_PTR stackSize = 1024 * 1024;
#endif

	GpFiberStarter_Win32::FiberStartState startState;
	startState.m_context = context;
	startState.m_creatingFiber = creatingFiber;
	startState.m_threadFunc = threadFunc;

	void *fiber = CreateFiber(static_cast<SIZE_T>(stackSize), GpFiberStarter_Win32::FiberStartRoutine, &startState);
	if (!fiber)
		return nullptr;

	SwitchToFiber(fiber);

	return GpFiber_Win32::Create(fiber);
}
