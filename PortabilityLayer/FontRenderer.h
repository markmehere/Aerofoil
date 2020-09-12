#pragma once

#include "FontHacks.h"

struct IGpFont;

namespace PortabilityLayer
{
	class RenderedFont;

	class FontRenderer
	{
	public:
		virtual RenderedFont *RenderFont(IGpFont *font, int size, bool aa, FontHacks fontHacks) = 0;

		static FontRenderer *GetInstance();
	};
}
