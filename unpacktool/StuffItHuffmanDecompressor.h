#pragma once

#include "IDecompressor.h"
#include "PrefixCode.h"

class StuffItHuffmanDecompressor : public IDecompressor
{
public:
	StuffItHuffmanDecompressor();
	~StuffItHuffmanDecompressor();

	bool Reset(CSInputBuffer *input, size_t compressedSize, size_t decompressedSize) override;
	bool ReadBytes(void *dest, size_t numBytes)  override;

private:
	CSInputBuffer *input;
	XADPrefixCode *code;

	bool parseTree();
	bool resetByteStream();
};
