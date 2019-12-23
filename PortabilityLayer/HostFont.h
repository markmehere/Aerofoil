#pragma once

#include <stdint.h>

namespace PortabilityLayer
{
	class HostFontRenderedGlyph;

	class HostFont
	{
	public:
		virtual void Destroy() = 0;
		virtual HostFontRenderedGlyph *Render(uint32_t unicodeCodePoint, unsigned int size) = 0;
	};
}
