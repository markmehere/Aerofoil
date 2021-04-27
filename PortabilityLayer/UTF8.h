#pragma once

#include <stdint.h>
#include <stddef.h>

namespace PortabilityLayer
{
	class UTF8Processor
	{
	public:
		static bool DecodeToMacRomanPascalStr(const uint8_t *inChars, size_t inSize, uint8_t *outChars, size_t maxOutSize, size_t &outSize);
	};
}
