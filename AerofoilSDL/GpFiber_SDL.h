#pragma once

#include "IGpFiber.h"
#include "SDL_thread.h"

namespace PortabilityLayer
{
	class HostSystemServices;
	class HostThreadEvent;
}

class GpFiber_SDL final : public IGpFiber
{
public:
	explicit GpFiber_SDL(SDL_Thread *thread, PortabilityLayer::HostThreadEvent *threadEvent);
	~GpFiber_SDL();

	void YieldTo(IGpFiber *fromFiber) override;
	void Destroy() override;

private:
	static int SDLCALL InternalThreadFunction(void *data);

	bool m_isDestroying;
	PortabilityLayer::HostThreadEvent *m_event;
	SDL_Thread *m_thread;
};
