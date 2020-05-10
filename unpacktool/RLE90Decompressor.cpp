#include "RLE90Decompressor.h"

#include "CSInputBuffer.h"

RLE90Decompressor::RLE90Decompressor()
	: m_input(nullptr)
	, m_repeatedByte(0)
	, m_count(0)
{
}

bool RLE90Decompressor::Reset(CSInputBuffer *input, size_t compressedSize, size_t decompressedSize)
{
	m_input = input;
	m_repeatedByte = 0;
	m_count = 0;
	return true;
}

bool RLE90Decompressor::ReadBytes(void *dest, size_t numBytes)
{
	uint8_t *destBytes = static_cast<uint8_t*>(dest);
	for (size_t i = 0; i < numBytes; i++)
	{
		if (m_count)
		{
			m_count--;
			destBytes[i] = m_repeatedByte;
		}
		else
		{
			bool isEOF;
			if (!CSInputAtEOF(m_input, isEOF))
				return false;

			if (isEOF)
				return false;

			int b;
			if (!CSInputNextByte(m_input, b))
				return false;

			if (b != 0x90)
			{
				destBytes[i] = b;
				m_repeatedByte = b;
			}
			else
			{
				int c;
				if (!CSInputNextByte(m_input, c))
					return false;

				if (c == 0)
				{
					m_repeatedByte = 0x90;
					destBytes[i] = 0x90;
				}
				else
				{
					if (c == 1)
						return false;
					m_count = c - 2;
					destBytes[i] = m_repeatedByte;
				}
			}
		}
	}

	return true;
}

