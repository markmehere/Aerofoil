#include "UTF16.h"

namespace PortabilityLayer
{
	bool UTF16Processor::DecodeCodePoint(const uint16_t *characters, size_t availableCharacters, size_t &outCharactersDigested, uint32_t &outCodePoint)
	{
		if (availableCharacters <= 0)
			return false;

		if ((characters[0] & 0xff80) == 0x00)
		{
			outCharactersDigested = 1;
			outCodePoint = characters[0];
			return true;
		}

		if (characters[0] <= 0xd7ff || characters[0] >= 0xe000)
		{
			outCharactersDigested = 1;
			outCodePoint = characters[0];
			return true;
		}

		// Surrogate pair
		if (characters[0] >= 0xdc00 || availableCharacters < 2)
			return false;

		if (characters[1] < 0xdc00 || characters[1] >= 0xe000)
			return false;

		uint16_t highBits = (characters[0] & 0x3ff);
		uint16_t lowBits = (characters[1] & 0x3ff);

		outCharactersDigested = 2;
		outCodePoint = (highBits << 10) + lowBits + 0x10000;

		return true;
	}

	void UTF16Processor::EncodeCodePoint(uint16_t *characters, size_t &outCharactersEmitted, uint32_t codePoint)
	{
		if (codePoint <= 0xd7ff || codePoint >= 0xe000)
		{
			outCharactersEmitted = 1;
			characters[0] = static_cast<uint16_t>(codePoint);
			return;
		}

		uint32_t codePointBits = (codePoint - 0x10000) & 0xfffff;
		uint16_t lowBits = (codePointBits & 0x3ff);
		uint16_t highBits = ((codePointBits >> 10) & 0x3ff);

		outCharactersEmitted = 2;
		characters[0] = (0xd800 + highBits);
		characters[1] = (0xdc00 + lowBits);
	}
}
