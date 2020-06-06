#pragma once

#include <stdint.h>

namespace PortabilityLayer
{
	struct RGBAColor;

	struct AntiAliasTable
	{
		// Striped 256x16 because constant background color is more likely than constant sample
		uint8_t m_aaTranslate[256][16];

		void GenerateForPalette(const RGBAColor &baseColor, const RGBAColor *colors, size_t numColors);
	};
}
