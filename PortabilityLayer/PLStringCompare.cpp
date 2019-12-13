#include "PLStringCompare.h"
#include "MacLatin.h"

#include <string.h>

Boolean EqualString(const PLPasStr &string1, const PLPasStr &string2, Boolean caseSensitive, Boolean diacriticSensitive)
{
	const size_t len = string1.Length();
	if (len != string2.Length())
		return PL_FALSE;

	const uint8_t *chars1 = string1.UChars();
	const uint8_t *chars2 = string2.UChars();

	if (caseSensitive)
	{
		// Case sensitive
		if (diacriticSensitive)
		{
			// Diacritic sensitive
			return memcmp(chars1, chars2, len) ? PL_FALSE : PL_TRUE;
		}
		else
		{
			// Diacritic insensitive
			for (size_t i = 0; i < len; i++)
			{
				const uint8_t c1 = chars1[i];
				const uint8_t c2 = chars2[i];

				if (PortabilityLayer::MacLatin::g_stripDiacritic[c1] != PortabilityLayer::MacLatin::g_stripDiacritic[c2])
					return PL_FALSE;
			}

			return PL_TRUE;
		}
	}
	else
	{
		// Case insensitive
		if (diacriticSensitive)
		{
			// Diacritic sensitive
			for (size_t i = 0; i < len; i++)
			{
				const uint8_t c1 = chars1[i];
				const uint8_t c2 = chars2[i];

				if (PortabilityLayer::MacLatin::g_toLower[c1] != PortabilityLayer::MacLatin::g_toLower[c2])
					return PL_FALSE;
			}

			return PL_TRUE;
		}
		else
		{
			// Diacritic insensitive
			for (size_t i = 0; i < len; i++)
			{
				const uint8_t c1 = PortabilityLayer::MacLatin::g_stripDiacritic[chars1[i]];
				const uint8_t c2 = PortabilityLayer::MacLatin::g_stripDiacritic[chars2[i]];

				if (PortabilityLayer::MacLatin::g_toLower[c1] != PortabilityLayer::MacLatin::g_toLower[c2])
					return PL_FALSE;
			}

			return PL_TRUE;
		}
	}
}
