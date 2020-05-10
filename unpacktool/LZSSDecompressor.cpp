#include "LZSSDecompressor.h"

#include <string.h>

bool LZSSDecompressor::ReadBytes(void *dest, size_t numBytes)
{
	for (size_t i = 0; i < numBytes; i++)
	{
		if (!ReadOneByte(static_cast<uint8_t*>(dest)[i]))
			return false;
	}

	return true;
}

bool LZSSDecompressor::Reset(CSInputBuffer *input, size_t compressedSize, size_t decompressedSize)
{
	this->input = input;

	windowbuffer = new uint8_t[windowsize];
	windowmask = windowsize - 1; // Assumes windows are always power-of-two sized!

	matchlength = 0;
	matchoffset = 0;
	memset(windowbuffer, 0, windowmask + 1);

	return this->resetLZSSHandle();
}

LZSSDecompressor::LZSSDecompressor(int windowsize)
	: windowbuffer(nullptr)
	, windowmask(0)
	, matchlength(0)
	, matchoffset(0)
	, input(nullptr)
	, pos(0)
	, windowsize(windowsize)
{
}

LZSSDecompressor::~LZSSDecompressor()
{
	delete[] windowbuffer;
}


bool LZSSDecompressor::ReadOneByte(uint8_t &b)
{
	if (!matchlength)
	{
		int offset, length;
		int val;
		if (!this->nextLiteralOrOffset(&offset, &length, val))
			return false;

		if (val >= 0)
		{
			windowbuffer[pos&windowmask] = val;
			b = val;
			pos++;
			return true;
		}
		else if (val == XADLZSSEnd)
		{
			return false;
		}
		else
		{
			matchlength = length;
			matchoffset = (int)(pos - offset);
		}
	}

	matchlength--;

	uint8_t byte = windowbuffer[matchoffset++&windowmask];

	windowbuffer[pos&windowmask] = byte;
	pos++;

	b = byte;
	return true;
}
