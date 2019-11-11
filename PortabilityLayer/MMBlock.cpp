#include "MMBlock.h"

namespace PortabilityLayer
{
	size_t MMBlock::AlignedSize()
	{
		const size_t paddedSize = sizeof(MMBlock) + PL_SYSTEM_MEMORY_ALIGNMENT - 1;
		const size_t paddedSizeTruncated = paddedSize - (paddedSize % PL_SYSTEM_MEMORY_ALIGNMENT);

		return paddedSizeTruncated;
	}
}
