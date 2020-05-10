#include "StringCommon.h"

#include "MacRomanConversion.h"
#include "UTF8.h"

void StringCommon::ConvertMacRomanFileName(std::vector<uint8_t> &utf8FileName, const uint8_t *macRomanName, size_t macRomanLength)
{
	for (size_t i = 0; i < macRomanLength; i++)
	{
		uint8_t bytes[8];
		size_t bytesEmitted;
		PortabilityLayer::UTF8Processor::EncodeCodePoint(bytes, bytesEmitted, MacRoman::ToUnicode(macRomanName[i]));

		for (size_t bi = 0; bi < bytesEmitted; bi++)
			utf8FileName.push_back(bytes[bi]);
	}
}
