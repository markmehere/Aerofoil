#pragma once

#include "FontHacks.h"
#include <stdint.h>

class PLPasStr;

namespace PortabilityLayer
{
	class HostFont;

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
		PortabilityLayer::HostFont *GetFontForVariation(int variation) const;
		PortabilityLayer::FontHacks GetHacksForVariation(int variation) const;

		static FontFamily *Create();
		void Destroy();

	private:
		PortabilityLayer::FontHacks m_hacks[kNumVariations];
		PortabilityLayer::HostFont *m_fonts[kNumVariations];
		uint8_t m_defaultVariation;

		FontFamily();
		~FontFamily();
	};
}
