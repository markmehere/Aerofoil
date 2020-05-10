#include "CompactProLZHDecompressor.h"

#include "CSInputBuffer.h"
#include "PrefixCode.h"

CompactProLZHDecompressor::CompactProLZHDecompressor(int blocksize)
	: LZSSDecompressor(8192)
	, literalcode(nullptr)
	, lengthcode(nullptr)
	, offsetcode(nullptr)
	, blocksize(blocksize)
	, blockcount(0)
	, blockstart(0)
{
}

CompactProLZHDecompressor::~CompactProLZHDecompressor()
{
	delete literalcode;
	delete lengthcode;
	delete offsetcode;
}

bool CompactProLZHDecompressor::resetLZSSHandle()
{
	blockcount = blocksize;
	blockstart = 0;

	return true;
}

bool CompactProLZHDecompressor::nextLiteralOrOffset(int *offset, int *length, int &result)
{
	if (blockcount >= blocksize)
	{
		if (blockstart)
		{
			// Don't let your bad implementations leak into your file formats, people!
			if (!CSInputSkipToByteBoundary(input))
				return false;

			if ((CSInputBufferOffset(input) - blockstart) & 1)
			{
				if (!CSInputSkipBytes(input, 3))
					return false;
			}
			else
			{
				if (!CSInputSkipBytes(input, 2))
					return false;
			}
		}

		delete literalcode;
		delete lengthcode;
		delete offsetcode;
		literalcode = lengthcode = offsetcode = nullptr;
		literalcode = allocAndParseCodeOfSize(256);
		if (!literalcode)
			return false;
		lengthcode = allocAndParseCodeOfSize(64);
		if (!lengthcode)
			return false;
		offsetcode = allocAndParseCodeOfSize(128);
		if (!offsetcode)
			return false;
		blockcount = 0;
		blockstart = CSInputBufferOffset(input);
	}

	unsigned int nextBit;
	if (!CSInputNextBit(input, nextBit))
		return false;

	if (nextBit)
	{
		blockcount += 2;
		return CSInputNextSymbolUsingCode(input, literalcode, result);
	}
	else
	{
		blockcount += 3;

		if (!CSInputNextSymbolUsingCode(input, lengthcode, *length))
			return false;

		int offsetSym;
		if (!CSInputNextSymbolUsingCode(input, offsetcode, offsetSym))
			return false;

		unsigned int offsetBits;
		if (!CSInputNextBitString(input, 6, offsetBits))
			return false;

		*offset = offsetSym << 6;
		*offset |= offsetBits;

		result = XADLZSSMatch;
		return true;
	}

	result = XADLZSSEnd;
	return true;
}

XADPrefixCode *CompactProLZHDecompressor::allocAndParseCodeOfSize(int size)
{
	int numbytes;
	if (!CSInputNextByte(input, numbytes))
		return nullptr;

	if (numbytes * 2 > size)
		return nullptr;

	std::vector<int> codelengths;
	codelengths.resize(size);

	for (int i = 0; i < numbytes; i++)
	{
		int val;
		if (!CSInputNextByte(input, val))
			return nullptr;
		codelengths[2 * i] = val >> 4;
		codelengths[2 * i + 1] = val & 0x0f;
	}
	for (int i = numbytes * 2; i < size; i++) codelengths[i] = 0;

	return XADPrefixCode::prefixCodeWithLengths(codelengths.data(), size, 15, true);
}
