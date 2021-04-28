#include "UTF8.h"
#include "GpUnicode.h"
#include "MacRomanConversion.h"

namespace PortabilityLayer
{
	bool UTF8Processor::DecodeToMacRomanPascalStr(const uint8_t *inChars, size_t inSize, uint8_t *outChars, size_t maxOutSize, size_t &outSizeRef)
	{
		size_t outSize = 0;
		while (inSize > 0 && outSize < maxOutSize)
		{
			size_t digestedChars = 0;
			uint32_t codePoint = 0;
			if (!GpUnicode::UTF8::Decode(inChars, inSize, digestedChars, codePoint))
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
					uint16_t decodedCP = MacRoman::ToUnicode(static_cast<uint8_t>(c));
					if (decodedCP == codePoint)
					{
						macRomanChar = static_cast<uint8_t>(c);
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
