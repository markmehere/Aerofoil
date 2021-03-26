#pragma once

#include "VirtualDirectory.h"

#include <stdint.h>

namespace PLSysCalls
{
	void Sleep(uint32_t ticks);
	void Exit(int exitCode);

	int MainExitWrapper(int (*mainFunc)());
}
