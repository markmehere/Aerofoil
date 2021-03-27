#pragma once

#include "FontFamilyID.h"
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
		bool GetFontSpec(int variation, FontHacks &outHacks, const char *&outPath);

		int GetVariationForFlags(int flags) const;
		IGpFont *GetFontForVariation(int variation);
		FontHacks GetHacksForVariation(int variation) const;

		FontFamilyID_t GetFamilyID() const;

		void PurgeCache();

		static FontFamily *Create(FontFamilyID_t familyID);
		void Destroy();

	private:
		struct FontSpec
		{
			FontSpec();

			IGpFont *m_font;
			FontHacks m_hacks;
			const char *m_fontPath;
			bool m_isRegistered;
		};

		FontSpec m_fontSpecs[kNumVariations];
		uint8_t m_defaultVariation;
		FontFamilyID_t m_familyID;

		explicit FontFamily(FontFamilyID_t familyID);
		~FontFamily();
	};
}
