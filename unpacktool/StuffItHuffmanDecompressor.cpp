#include "StuffItHuffmanDecompressor.h"

StuffItHuffmanDecompressor::StuffItHuffmanDecompressor()
	: input(nullptr)
	, code(nullptr)
{
}

StuffItHuffmanDecompressor::~StuffItHuffmanDecompressor()
{
	delete code;
}

bool StuffItHuffmanDecompressor::Reset(CSInputBuffer *input, size_t compressedSize, size_t decompressedSize)
{
	this->input = input;
	code = nullptr;

	return this->resetByteStream();
}

bool StuffItHuffmanDecompressor::resetByteStream()
{
	delete code;
	code = XADPrefixCode::prefixCode();

	code->startBuildingTree();
	if (!parseTree())
		return false;

	return true;
}

bool StuffItHuffmanDecompressor::parseTree()
{
	unsigned int firstBit;
	if (!CSInputNextBit(input, firstBit))
		return false;

	if (firstBit == 1)
	{
		unsigned int nextBits;
		if (!CSInputNextBitString(input, 8, nextBits))
			return false;
		code->makeLeafWithValue(nextBits);
	}
	else
	{
		code->startZeroBranch();
		if (!parseTree())
			return false;

		code->startOneBranch();
		if (!parseTree())
			return false;

		code->finishBranches();
	}

	return true;
}

bool StuffItHuffmanDecompressor::ReadBytes(void *dest, size_t numBytes)
{
	for (size_t i = 0; i < numBytes; i++)
	{
		int sym;
		if (!CSInputNextSymbolUsingCode(input, code, sym))
			return false;

		static_cast<uint8_t*>(dest)[i] = sym;
	}

	return true;
}
