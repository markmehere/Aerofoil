#pragma once

#include "FontHacks.h"

namespace PortabilityLayer
{
	class RenderedFont;
	class HostFont;

	class FontRenderer
	{
	public:
		virtual RenderedFont *RenderFont(HostFont *font, int size, bool aa, FontHacks fontHacks) = 0;

		static FontRenderer *GetInstance();
	};
}
