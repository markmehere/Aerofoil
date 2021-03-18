#pragma once

#include <stdint.h>
#include <stddef.h>

namespace PortabilityLayer
{
	class ScanlineMask;
	struct Vec2i;
	struct Rect2i;

	class ScanlineMaskConverter
	{
	public:
		static ScanlineMask *CompilePoly(const Vec2i *points, size_t numPoints);
		static ScanlineMask *CompileEllipse(const Rect2i &rect);
	};
}
