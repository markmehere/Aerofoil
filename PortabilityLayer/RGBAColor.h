#pragma once

#include <stdint.h>

namespace PortabilityLayer
{
	struct RGBAColor
	{
		uint8_t r, g, b, a;

		bool operator==(const RGBAColor &other) const;
		bool operator!=(const RGBAColor &other) const;
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

	inline bool RGBAColor::operator==(const RGBAColor &other) const
	{
		return this->r == other.r && this->g == other.g && this->b == other.b && this->a == other.a;
	}

	inline bool RGBAColor::operator!=(const RGBAColor &other) const
	{
		return !((*this) == other);
	}
}
