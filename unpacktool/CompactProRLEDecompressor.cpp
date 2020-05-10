#include "CompactProRLEDecompressor.h"

#include "CSInputBuffer.h"

CompactProRLEDecompressor::CompactProRLEDecompressor()
	: input(nullptr)
	, saved(0)
	, repeat(0)
	, halfescaped(false)
{
}

CompactProRLEDecompressor::~CompactProRLEDecompressor()
{
}

bool CompactProRLEDecompressor::Reset(CSInputBuffer *input, size_t compressedSize, size_t decompressedSize)
{
	saved = 0;
	repeat = 0;
	halfescaped = false;
	this->input = input;

	return true;
}

bool CompactProRLEDecompressor::ReadBytes(void *dest, size_t numBytes)
{
	uint8_t *destBytes = static_cast<uint8_t*>(dest);

	while (numBytes)
	{
		if (!EmitOneByte(*destBytes))
			return false;

		numBytes--;
		destBytes++;
	}

	return true;
}

bool CompactProRLEDecompressor::EmitOneByte(uint8_t &b)
{
	if (repeat)
	{
		repeat--;
		b = saved;
		return true;
	}

	int byte;
	if (halfescaped)
	{
		byte = 0x81;
		halfescaped = true;
	}
	else
	{
		if (!CSInputNextByte(input, byte))
			return false;
	}

	if (byte == 0x81)
	{
		if (!CSInputNextByte(input, byte))
			return false;

		if (byte == 0x82)
		{
			if (!CSInputNextByte(input, byte))
				return false;

			if (byte != 0)
			{
				repeat = byte - 2; // ?
				b = saved;
				return true;
			}
			else
			{
				repeat = 1;
				saved = 0x82;
				b = 0x81;
				return true;
			}
		}
		else
		{
			if (byte == 0x81)
			{
				halfescaped = true;
				saved = 0x81;
				b = saved;
				return true;
			}
			else
			{
				repeat = 1;
				saved = byte;
				b = 0x81;
				return true;
			}
		}
	}
	else
	{
		saved = byte;
		b = saved;
		return true;
	}
}
