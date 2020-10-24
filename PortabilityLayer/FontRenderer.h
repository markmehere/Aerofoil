#pragma once

#include "FontHacks.h"

struct IGpFont;
class GpIOStream;

namespace PortabilityLayer
{
	class RenderedFont;

	class FontRenderer
	{
	public:
		virtual RenderedFont *RenderFont(IGpFont *font, int size, bool aa, FontHacks fontHacks) = 0;
		virtual RenderedFont *LoadCache(GpIOStream *stream) = 0;
		virtual bool SaveCache(const RenderedFont *rfont, GpIOStream *stream) = 0;

		static FontRenderer *GetInstance();
	};
}
