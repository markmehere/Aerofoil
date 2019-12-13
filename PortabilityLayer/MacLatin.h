#pragma once

#include <stdint.h>

namespace PortabilityLayer
{
	namespace MacLatin
	{
		extern const uint16_t g_toUnicode[256];
		extern const uint8_t g_stripDiacritic[256];
		extern const uint8_t g_toLower[256];
		extern const uint8_t g_toUpper[256];
	}
}
