#pragma once

#include "CoreDefs.h"

struct IGpFiber
{
	virtual void YieldTo() = 0;
	virtual void Destroy() = 0;
};
