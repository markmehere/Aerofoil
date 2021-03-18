#pragma once

#include "IGpFiber.h"

struct IGpThreadEvent;

class GpFiber_Thread final : public IGpFiber
{
public:
	explicit GpFiber_Thread(void *thread, IGpThreadEvent *threadEvent);
	~GpFiber_Thread();

	void YieldTo(IGpFiber *fromFiber) override;
	void YieldToTerminal(IGpFiber *fromFiber) override;
	void Destroy() override;

private:
	static int InternalThreadFunction(void *data);

	bool m_isDestroying;
	IGpThreadEvent *m_event;
	void *m_thread;
};
