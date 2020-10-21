#pragma once

#include "VirtualDirectory.h"

#include <stdint.h>

namespace PLSysCalls
{
	void Sleep(uint32_t ticks);
	void ForceSyncFrame();
	void RunOnVOSThread(void(*callback)(void *context), void *context);
}
