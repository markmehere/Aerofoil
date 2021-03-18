#pragma once

#include <stdint.h>
#include <stddef.h>

namespace PortabilityLayer
{
	class UTF8Processor
	{
	public:
		static bool DecodeCodePoint(const uint8_t *characters, size_t availableCharacters, size_t &outCharactersDigested, uint32_t &outCodePoint);
		static void EncodeCodePoint(uint8_t *characters, size_t &outCharactersEmitted, uint32_t codePoint);

		static bool DecodeToMacRomanPascalStr(const uint8_t *inChars, size_t inSize, uint8_t *outChars, size_t maxOutSize, size_t &outSize);

		static const unsigned int kMaxEncodedBytes = 4;
	};
}
