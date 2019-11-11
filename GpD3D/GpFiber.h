#pragma once

#include "CoreDefs.h"

class GpFiber
{
public:
	virtual void YieldTo() = 0;
	virtual void Destroy() = 0;
};
