#pragma once

#include "IArchiveParser.h"

class StuffIt5Parser : public IArchiveParser
{
public:
	bool Check(IFileReader &reader) override;
	ArchiveItemList *Parse(IFileReader &reader) override;
};
