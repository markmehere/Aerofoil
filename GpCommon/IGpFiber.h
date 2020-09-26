#pragma once

#include "CoreDefs.h"

struct IGpFiber
{
	virtual void YieldTo(IGpFiber *toFiber) = 0;
	virtual void Destroy() = 0;
};
