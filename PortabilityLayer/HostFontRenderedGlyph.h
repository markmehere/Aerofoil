#pragma once

namespace PortabilityLayer
{
	struct RenderedGlyphMetrics;

	class HostFontRenderedGlyph
	{
	public:
		virtual const RenderedGlyphMetrics &GetMetrics() const = 0;
		virtual const void *GetData() const = 0;
		virtual void Destroy() = 0;
	};
}
