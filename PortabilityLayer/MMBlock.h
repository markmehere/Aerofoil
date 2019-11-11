#pragma once
#ifndef __PL_MM_BLOCK_H__
#define __PL_MM_BLOCK_H__

#include <stdint.h>

#include "CoreDefs.h"
#include "SmallestInt.h"

namespace PortabilityLayer
{
	struct MMBlock
	{
		SmallestUInt<PL_SYSTEM_MEMORY_ALIGNMENT>::ValueType_t m_offsetFromAllocLocation;

		static size_t AlignedSize();
	};
}

#endif
