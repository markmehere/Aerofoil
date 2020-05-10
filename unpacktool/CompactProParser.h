#pragma once

#include "IArchiveParser.h"

#include <stdint.h>

class CompactProParser : public IArchiveParser
{
public:
	bool Check(IFileReader &reader) override;
	ArchiveItemList *Parse(IFileReader &reader) override;

private:
	ArchiveItemList *ParseDirectory(uint32_t numEntries, IFileReader &reader);
};
