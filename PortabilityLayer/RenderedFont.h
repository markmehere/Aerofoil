#pragma once

#include <stdint.h>

namespace PortabilityLayer
{
	struct RenderedFontMetrics;

	struct RenderedGlyphMetrics;

	class RenderedFont
	{
	public:
		virtual bool GetGlyph(unsigned int character, const RenderedGlyphMetrics *&outMetricsPtr, const void *&outData) const = 0;
		virtual const RenderedFontMetrics &GetMetrics() const = 0;
		virtual size_t MeasureString(const uint8_t *chars, size_t len) const = 0;
		virtual bool IsAntiAliased() const = 0;

		virtual void Destroy() = 0;
	};
}
