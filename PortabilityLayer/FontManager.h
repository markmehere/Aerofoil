#pragma once

#include "FontHacks.h"
#include "FontFamilyID.h"

struct IGpFont;

namespace PortabilityLayer
{
	class FontFamily;
	class RenderedFont;

	namespace FontPresets
	{
		enum FontPreset
		{
			kSystem12,
			kSystem12Bold,

			kApplication8,
			kApplication9,
			kApplication9Bold,
			kApplication10Bold,
			kApplication12Bold,
			kApplication14,
			kApplication14Bold,
			kApplication18,
			kApplication40,

			kMono10,

			kHandwriting24,
			kHandwriting48,

			kCount,
		};
	}

	typedef FontPresets::FontPreset FontPreset_t;

	class FontManager
	{
	public:
		virtual void Init() = 0;
		virtual void Shutdown() = 0;

		virtual FontFamily *GetFont(FontFamilyID_t fontFamilyID) const = 0;
		virtual void GetFontPreset(FontPreset_t fontPreset, FontFamilyID_t *outFamilyID, int *outSize, int *outVariationFlags, bool *outAA) const = 0;

		virtual RenderedFont *GetRenderedFontFromFamily(FontFamily *fontFamily, int fontSize, bool aa, int flags) = 0;

		virtual RenderedFont *LoadCachedRenderedFont(FontFamilyID_t familyID, int size, bool aa, int flags) = 0;

		virtual void PurgeCache() = 0;

		static FontManager *GetInstance();
	};
}
