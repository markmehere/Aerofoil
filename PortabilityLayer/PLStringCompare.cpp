#include "PLStringCompare.h"
#include "MacRomanConversion.h"

#include <string.h>
#include <algorithm>

namespace
{
	static uint8_t ToLowerInvariant(uint8_t v)
	{
		// This isn't very comprehensive, but we're only ever using this for two cases that don't contain any diacritics or anything
		if (v >= 65 && v <= 90)
			return v + 32;
		else
			return v;
	}
}

namespace StrCmp
{
	int Compare(const PLPasStr &string1, const PLPasStr &string2)
	{
		const uint8_t *chars1 = string1.UChars();
		const uint8_t *chars2 = string2.UChars();

		const size_t len1 = string1.Length();
		const size_t len2 = string1.Length();

		const size_t shorterLen = std::min(len1, len2);

		int memcmpResult = memcmp(chars1, chars2, shorterLen);

		if (memcmpResult != 0)
			return memcmpResult;

		if (len1 < len2)
			return -1;
		else if (len2 < len1)
			return 1;
		else
			return 0;
	}

	int CompareCaseInsensitive(const PLPasStr &string1, const PLPasStr &string2)
	{
		const uint8_t *chars1 = string1.UChars();
		const uint8_t *chars2 = string2.UChars();

		const size_t len1 = string1.Length();
		const size_t len2 = string1.Length();

		const size_t shorterLen = std::min(len1, len2);

		for (size_t i = 0; i < shorterLen; i++)
		{
			const uint8_t c1 = ToLowerInvariant(chars1[i]);
			const uint8_t c2 = ToLowerInvariant(chars2[i]);

			if (c1 < c2)
				return -1;
			if (c2 < c1)
				return 1;
		}

		if (len1 < len2)
			return -1;
		else if (len2 < len1)
			return 1;
		else
			return 0;
	}
}
