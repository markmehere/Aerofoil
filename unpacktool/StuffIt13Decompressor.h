#pragma once

#include "LZSSDecompressor.h"
#include "IFileReader.h"

class XADPrefixCode;

class StuffIt13Decompressor : public LZSSDecompressor
{
public:
	StuffIt13Decompressor();
	~StuffIt13Decompressor();

private:
	XADPrefixCode *firstcodei, *secondcodei, *offsetcodei;
	XADPrefixCode *firstcoder, *secondcoder, *offsetcoder;
	XADPrefixCode *currcode;

	bool resetLZSSHandle();
	XADPrefixCode *allocAndParseCodeOfSize(int numcodes, XADPrefixCode *metacode);
	bool nextLiteralOrOffset(int *offset, int *length, int &result);
};
