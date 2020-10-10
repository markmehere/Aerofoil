#pragma once

struct IGpFiber;

namespace PortabilityLayer
{
	class HostSystemServices;
}

class GpFiberStarter
{
public:
	typedef void(*ThreadFunc_t)(void *context);

	static IGpFiber *StartFiber(PortabilityLayer::HostSystemServices *systemServices, ThreadFunc_t threadFunc, void *context, IGpFiber *creatingFiber);
};
