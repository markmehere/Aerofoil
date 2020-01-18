#pragma once

#include <stdint.h>

namespace PortabilityLayer
{
	class HostFontRenderedGlyph;
	struct RenderedFontMetrics;

	class HostFont
	{
	public:
		virtual void Destroy() = 0;
		virtual HostFontRenderedGlyph *Render(uint32_t unicodeCodePoint, unsigned int size, bool aa) = 0;
		virtual bool GetLineSpacing(unsigned int size, int32_t &outSpacing) = 0;
	};
}
