#pragma once

struct IGpFiber;

class GpFiberStarter
{
public:
	typedef void(*ThreadFunc_t)(void *context);

	static IGpFiber *StartFiber(ThreadFunc_t threadFunc, void *context, IGpFiber *creatingFiber);
};
