#pragma once

#include "EGpFontHandlerType.h"

struct IGpAllocator;

struct GpFontHandlerProperties
{
	EGpFontHandlerType m_type;

	IGpAllocator *m_alloc;
};
