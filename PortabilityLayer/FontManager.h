#pragma once

#include "FontHacks.h"

struct IGpFont;

namespace PortabilityLayer
{
	class FontFamily;
	class RenderedFont;

	class FontManager
	{
	public:
		virtual void Init() = 0;
		virtual void Shutdown() = 0;

		virtual FontFamily *GetSystemFont(int fontSize, int variationFlags) const = 0;
		virtual FontFamily *GetApplicationFont(int fontSize, int variationFlags) const = 0;
		virtual FontFamily *GetHandwritingFont(int fontSize, int variationFlags) const = 0;
		virtual FontFamily *GetMonospaceFont(int fontSize, int variationFlags) const = 0;

		virtual RenderedFont *GetRenderedFontFromFamily(FontFamily *fontFamily, int fontSize, bool aa, int flags) = 0;

		virtual RenderedFont *LoadCachedRenderedFont(int cacheID, int size, bool aa, int flags) const = 0;
		virtual void SaveCachedRenderedFont(const RenderedFont *rfont, int cacheID, int size, bool aa, int flags) const = 0;

		virtual void PurgeCache() = 0;

		static FontManager *GetInstance();
	};
}
