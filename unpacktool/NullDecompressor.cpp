#include "NullDecompressor.h"
#include "CSInputBuffer.h"

NullDecompressor::NullDecompressor()
	: m_input(nullptr)
{
}

bool NullDecompressor::Reset(CSInputBuffer *input, size_t compressedSize, size_t decompressedSize)
{
	m_input = input;
	return true;
}

bool NullDecompressor::ReadBytes(void *dest, size_t numBytes)
{
	uint8_t *output = static_cast<uint8_t *>(dest);

	for (size_t i = 0; i < numBytes; i++)
	{
		int byte;
		if (!CSInputNextByte(m_input, byte))
			return false;

		output[i] = byte;
	}

	return true;
}
