#include "MMBlock.h"

namespace PortabilityLayer
{
	size_t MMBlock::AlignedSize()
	{
		const size_t paddedSize = sizeof(MMBlock) + GP_SYSTEM_MEMORY_ALIGNMENT - 1;
		const size_t paddedSizeTruncated = paddedSize - (paddedSize % GP_SYSTEM_MEMORY_ALIGNMENT);

		return paddedSizeTruncated;
	}
}
