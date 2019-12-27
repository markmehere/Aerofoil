#pragma once

#include <stdint.h>

namespace PortabilityLayer
{
	struct RGBAColor
	{
		uint8_t r, g, b, a;

		static RGBAColor Create(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
	};

	inline RGBAColor RGBAColor::Create(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	{
		RGBAColor color;
		color.r = r;
		color.g = g;
		color.b = b;
		color.a = a;

		return color;
	}
}
