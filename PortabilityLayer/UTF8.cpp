#include "UTF8.h"
#include "MacRomanConversion.h"

namespace PortabilityLayer
{
	bool UTF8Processor::DecodeCodePoint(const uint8_t *characters, size_t availableCharacters, size_t &outCharactersDigested, uint32_t &outCodePoint)
	{
		if (availableCharacters <= 0)
			return false;

		if ((characters[0] & 0x80) == 0x00)
		{
			outCharactersDigested = 1;
			outCodePoint = characters[0];
			return true;
		}

		size_t sz = 0;
		uint32_t codePoint = 0;
		uint32_t minCodePoint = 0;
		if ((characters[0] & 0xe0) == 0xc0)
		{
			sz = 2;
			minCodePoint = 0x80;
			codePoint = (characters[0] & 0x1f);
		}
		else if ((characters[0] & 0xf0) == 0xe0)
		{
			sz = 3;
			minCodePoint = 0x800;
			codePoint = (characters[0] & 0x0f);
		}
		else if ((characters[0] & 0xf8) == 0xf0)
		{
			sz = 4;
			minCodePoint = 0x10000;
			codePoint = (characters[0] & 0x07);
		}
		else
			return false;

		if (availableCharacters < sz)
			return false;

		for (size_t auxByte = 1; auxByte < sz; auxByte++)
		{
			if ((characters[auxByte] & 0xc0) != 0x80)
				return false;

			codePoint = (codePoint << 6) | (characters[auxByte] & 0x3f);
		}

		if (codePoint < minCodePoint || codePoint > 0x10ffff)
			return false;

		if (codePoint >= 0xd800 && codePoint <= 0xdfff)
			return false;

		outCodePoint = codePoint;
		outCharactersDigested = sz;

		return true;
	}

	void UTF8Processor::EncodeCodePoint(uint8_t *characters, size_t &outCharactersEmitted, uint32_t codePoint)
	{
		codePoint &= 0x1fffff;

		uint8_t signalBits = 0;
		size_t numBytes = 0;
		if (codePoint < 0x0080)
		{
			numBytes = 1;
			signalBits = 0;
		}
		else if (codePoint < 0x0800)
		{
			numBytes = 2;
			signalBits = 0xc0;
		}
		else if (codePoint < 0x10000)
		{
			numBytes = 3;
			signalBits = 0xe0;
		}
		else
		{
			numBytes = 4;
			signalBits = 0xf0;
		}

		characters[0] = static_cast<uint8_t>((codePoint >> (6 * (numBytes - 1))) | signalBits);

		for (size_t i = 1; i < numBytes; i++)
		{
			const uint32_t isolate = ((codePoint >> (6 * (numBytes - 1 - i))) & 0x3f) | 0x80;
			characters[i] = static_cast<uint8_t>(isolate);
		}

		outCharactersEmitted = numBytes;
	}

	bool UTF8Processor::DecodeToMacRomanPascalStr(const uint8_t *inChars, size_t inSize, uint8_t *outChars, size_t maxOutSize, size_t &outSizeRef)
	{
		size_t outSize = 0;
		while (inSize > 0 && outSize < maxOutSize)
		{
			size_t digestedChars = 0;
			uint32_t codePoint = 0;
			if (!DecodeCodePoint(inChars, inSize, digestedChars, codePoint))
				return false;

			inChars += digestedChars;
			inSize -= digestedChars;

			uint8_t macRomanChar = 0;
			if (codePoint >= 0x11 && codePoint <= 0x14)
				macRomanChar = static_cast<uint8_t>('?');
			else if (codePoint < 0x80)
				macRomanChar = static_cast<uint8_t>(codePoint);
			else
			{
				for (uint16_t c = 0x80; c <= 0xff; c++)
				{
					uint16_t decodedCP = MacRoman::ToUnicode(c);
					if (decodedCP == codePoint)
					{
						macRomanChar = c;
						break;
					}
				}

				if (macRomanChar == 0)
					macRomanChar = static_cast<uint8_t>('?');
			}

			*outChars = macRomanChar;
			outChars++;
			outSize++;
		}

		outSizeRef = outSize;

		return true;
	}
}
