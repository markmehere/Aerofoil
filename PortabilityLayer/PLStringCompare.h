#pragma once

#include "PLCore.h"
#include "PLPasStr.h"

namespace StrCmp
{
	int CompareCaseInsensitive(const PLPasStr &string1, const PLPasStr &string2);
	int Compare(const PLPasStr &string1, const PLPasStr &string2);

	inline bool EqualCaseInsensitive(const PLPasStr &string1, const PLPasStr &string2)
	{
		return CompareCaseInsensitive(string1, string2) == 0;
	}

	inline bool Equal(const PLPasStr &string1, const PLPasStr &string2)
	{
		return Compare(string1, string2) == 0;
	}
}
