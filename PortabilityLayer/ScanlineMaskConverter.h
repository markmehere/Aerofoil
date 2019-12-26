#pragma once

#include <stdint.h>

namespace PortabilityLayer
{
	class ScanlineMask;
	struct Vec2i;

	class ScanlineMaskConverter
	{
	public:
		static ScanlineMask *CompilePoly(const Vec2i *points, size_t numPoints);
	};
}
