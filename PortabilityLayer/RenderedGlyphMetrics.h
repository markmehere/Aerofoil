#pragma once

#include <stdint.h>

namespace PortabilityLayer
{
	struct RenderedGlyphMetrics
	{
		size_t m_glyphDataPitch;

		uint32_t m_glyphWidth;
		uint32_t m_glyphHeight;

		int16_t m_bearingX;
		int16_t m_bearingY;
		int16_t m_advanceX;
	};
}
