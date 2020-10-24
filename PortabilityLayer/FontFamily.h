#pragma once

#include "FontHacks.h"
#include <stdint.h>

class PLPasStr;
struct IGpFont;

namespace PortabilityLayer
{
	enum FontFamilyFlags
	{
		FontFamilyFlag_None = 0,

		FontFamilyFlag_Bold = 1,

		FontFamilyFlag_All = 1,
	};

	class FontFamily final
	{
	public:
		static const unsigned int kNumVariations = FontFamilyFlag_All + 1;

		void AddFont(int flags, const char *path, FontHacks fontHacks);
		void SetDefaultVariation(int defaultVariation);

		int GetVariationForFlags(int variation) const;
		IGpFont *GetFontForVariation(int variation) const;
		FontHacks GetHacksForVariation(int variation) const;

		int GetCacheID() const;

		static FontFamily *Create(int cacheID);
		void Destroy();

	private:
		FontHacks m_hacks[kNumVariations];
		IGpFont *m_fonts[kNumVariations];
		uint8_t m_defaultVariation;
		int m_cacheID;

		explicit FontFamily(int cacheID);
		~FontFamily();
	};
}
