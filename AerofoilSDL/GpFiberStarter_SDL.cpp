#include "GpFiberStarter.h"
#include "GpFiber_SDL.h"

#include "HostSystemServices.h"
#include "HostThreadEvent.h"

#include "SDL_thread.h"

#include <assert.h>

namespace GpFiberStarter_SDL
{
	struct FiberStartState
	{
		GpFiberStarter::ThreadFunc_t m_threadFunc;
		PortabilityLayer::HostThreadEvent *m_creatingReturnEvent;
		PortabilityLayer::HostThreadEvent *m_creatingWakeEvent;
		void *m_context;
	};

	static int SDLCALL FiberStartRoutine(void *lpThreadParameter)
	{
		const FiberStartState *tss = static_cast<const FiberStartState*>(lpThreadParameter);

		GpFiberStarter::ThreadFunc_t threadFunc = tss->m_threadFunc;
		PortabilityLayer::HostThreadEvent *creatingReturnEvent = tss->m_creatingReturnEvent;
		PortabilityLayer::HostThreadEvent *wakeEvent = tss->m_creatingWakeEvent;
		void *context = tss->m_context;
		creatingReturnEvent->Signal();

		wakeEvent->Wait();

		threadFunc(context);

		return 0;
	}
}

IGpFiber *GpFiberStarter::StartFiber(PortabilityLayer::HostSystemServices *systemServices, ThreadFunc_t threadFunc, void *context, IGpFiber *creatingFiber)
{
	PortabilityLayer::HostThreadEvent *returnEvent = systemServices->CreateThreadEvent(true, false);
	if (!returnEvent)
		return nullptr;

	PortabilityLayer::HostThreadEvent *wakeEvent = systemServices->CreateThreadEvent(true, false);
	if (!wakeEvent)
	{
		returnEvent->Destroy();
		return nullptr;
	}

	GpFiberStarter_SDL::FiberStartState startState;
	startState.m_context = context;
	startState.m_creatingReturnEvent = returnEvent;
	startState.m_creatingWakeEvent = wakeEvent;
	startState.m_threadFunc = threadFunc;

	SDL_Thread *thread = SDL_CreateThread(GpFiberStarter_SDL::FiberStartRoutine, "Fiber", &startState);
	if (!thread)
	{
		returnEvent->Destroy();
		wakeEvent->Destroy();
		return nullptr;
	}

	returnEvent->Wait();
	returnEvent->Destroy();

	return new GpFiber_SDL(thread, wakeEvent);
}
