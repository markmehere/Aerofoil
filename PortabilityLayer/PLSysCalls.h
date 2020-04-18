#pragma once

#include "VirtualDirectory.h"

#include <stdint.h>

namespace PLSysCalls
{
	void Sleep(uint32_t ticks);
	bool PromptOpenFile(PortabilityLayer::VirtualDirectory_t dirID, char *path, size_t &outPathLength, size_t pathCapacity);
	bool PromptSaveFile(PortabilityLayer::VirtualDirectory_t virtualDirectory, char *path, size_t &outPathLength, size_t pathCapacity, const char *initialFileName);
}
