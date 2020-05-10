#pragma once

#include <stdint.h>

namespace PortabilityLayer
{
	class UTF16Processor
	{
	public:
		static bool DecodeCodePoint(const uint16_t *characters, size_t availableCharacters, size_t &outCharactersDigested, uint32_t &outCodePoint);
		static void EncodeCodePoint(uint16_t *characters, size_t &outCharactersEmitted, uint32_t codePoint);
	};
}
