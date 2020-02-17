#pragma once

#include <stdint.h>

namespace MacRoman
{
	uint16_t ToUnicode(uint8_t character);
	bool FromUnicode(uint8_t &outChar, uint16_t codePoint);
}
