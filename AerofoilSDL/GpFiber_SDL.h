#pragma once

#include "IGpFiber.h"
#include "SDL_thread.h"

struct IGpThreadEvent;

class GpFiber_SDL final : public IGpFiber
{
public:
	explicit GpFiber_SDL(SDL_Thread *thread, IGpThreadEvent *threadEvent);
	~GpFiber_SDL();

	void YieldTo(IGpFiber *fromFiber) override;
	void YieldToTerminal(IGpFiber *fromFiber) override;
	void Destroy() override;

private:
	static int SDLCALL InternalThreadFunction(void *data);

	bool m_isDestroying;
	IGpThreadEvent *m_event;
	SDL_Thread *m_thread;
};
