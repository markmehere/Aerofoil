#pragma once

#ifndef __PL_PASCALSTR_H__
#define __PL_PASCALSTR_H__

#include "UnsafePascalStr.h"

namespace PortabilityLayer
{
	template<size_t TSize>
	class PascalStr : public UnsafePascalStr<TSize, true>
	{
	public:
		PascalStr();
		PascalStr(size_t size, const char *str);
	};
}

#include <string.h>

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
}

#endif
