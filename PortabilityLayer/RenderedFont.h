#pragma once

namespace PortabilityLayer
{
	struct RenderedGlyphMetrics;

	class RenderedFont
	{
	public:
		virtual bool GetGlyph(unsigned int character, const RenderedGlyphMetrics **outMetricsPtr, const void **outData) const = 0;

		virtual void Destroy() = 0;
	};
}
