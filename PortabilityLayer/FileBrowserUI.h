#pragma once

#include <stdint.h>

#include "VirtualDirectory.h"

class PLPasStr;

namespace PortabilityLayer
{
	class FileBrowserUI
	{
	public:
		enum Mode
		{
			Mode_Save,
			Mode_Open,
		};

		static bool Prompt(Mode mode, VirtualDirectory_t dirID, char *path, size_t &outPathLength, size_t pathCapacity, const PLPasStr &initialFileName, const PLPasStr &promptText);
	};
}
