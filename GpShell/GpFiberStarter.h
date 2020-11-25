#pragma once

struct IGpFiber;
struct IGpSystemServices;

class GpFiberStarter
{
public:
	typedef void(*ThreadFunc_t)(void *context);

	static IGpFiber *StartFiber(IGpSystemServices *systemServices, ThreadFunc_t threadFunc, void *context, IGpFiber *creatingFiber);
};
