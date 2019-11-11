#pragma once

class GpFiber;

class GpFiberStarter
{
public:
	typedef void(*ThreadFunc_t)(void *context);

	static GpFiber *StartFiber(ThreadFunc_t threadFunc, void *context, GpFiber *creatingFiber);
};
