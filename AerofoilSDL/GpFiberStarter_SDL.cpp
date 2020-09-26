#include "GpFiberStarter.h"
#include "GpFiber_SDL.h"
#include "GpSystemServices_Win32.h"

#include "HostThreadEvent.h"

#include "SDL_thread.h"

#include <assert.h>

namespace GpFiberStarter_SDL
{
	struct FiberStartState
	{
		GpFiberStarter::ThreadFunc_t m_threadFunc;
		PortabilityLayer::HostThreadEvent *m_creatingReturnEvent;
		void *m_context;
	};

	static int SDLCALL FiberStartRoutine(void *lpThreadParameter)
	{
		const FiberStartState *tss = static_cast<const FiberStartState*>(lpThreadParameter);

		GpFiberStarter::ThreadFunc_t threadFunc = tss->m_threadFunc;
		PortabilityLayer::HostThreadEvent *creatingReturnEvent = tss->m_creatingReturnEvent;
		void *context = tss->m_context;
		creatingReturnEvent->Signal();

		threadFunc(context);

		return 0;
	}
}

IGpFiber *GpFiberStarter::StartFiber(ThreadFunc_t threadFunc, void *context, IGpFiber *creatingFiber)
{
	PortabilityLayer::HostThreadEvent *returnEvent = GpSystemServices_Win32::GetInstance()->CreateThreadEvent(true, false);

	GpFiberStarter_SDL::FiberStartState startState;
	startState.m_context = context;
	startState.m_creatingReturnEvent = returnEvent;
	startState.m_threadFunc = threadFunc;

	SDL_Thread *thread = SDL_CreateThread(GpFiberStarter_SDL::FiberStartRoutine, "Fiber", &startState);
	if (!thread)
		return nullptr;

	returnEvent->Wait();

	return new GpFiber_SDL(thread, returnEvent);
}
