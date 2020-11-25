#pragma once

#include <stdint.h>

struct IGpThreadEvent
{
public:
	virtual void Wait() = 0;
	virtual bool WaitTimed(uint32_t msec) = 0;
	virtual void Signal() = 0;
	virtual void Destroy() = 0;
};
