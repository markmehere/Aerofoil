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

		uint32_t AsUInt32() const;
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

	inline uint32_t RGBAColor::AsUInt32() const
	{
		uint32_t rgbaColor = 0;
		uint8_t *rgbaColorBytes = reinterpret_cast<uint8_t*>(&rgbaColor);
		rgbaColorBytes[0] = r;
		rgbaColorBytes[1] = g;
		rgbaColorBytes[2] = b;
		rgbaColorBytes[3] = a;
		return rgbaColor;
	}
}
