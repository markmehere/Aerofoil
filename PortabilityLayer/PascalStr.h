#pragma once

#include "UnsafePascalStr.h"

class PLPasStr;

namespace PortabilityLayer
{
	template<size_t TSize>
	class PascalStr : public UnsafePascalStr<TSize, true>
	{
	public:
		PascalStr();
		PascalStr(size_t size, const char *str);
		explicit PascalStr(const PLPasStr &pstr);
	};
}

#include <string.h>
#include "PLPasStr.h"

namespace PortabilityLayer
{
	template<size_t TSize>
	inline PascalStr<TSize>::PascalStr()
		: UnsafePascalStr<TSize, true>(0, nullptr)
	{
	}

	template<size_t TSize>
	PascalStr<TSize>::PascalStr(size_t size, const char *str)
		: UnsafePascalStr<TSize, true>(size, str)
	{
	}

	template<size_t TSize>
	PascalStr<TSize>::PascalStr(const PLPasStr &pstr)
		: UnsafePascalStr<TSize, true>(pstr.Length(), pstr.Chars())
	{
	}
}
