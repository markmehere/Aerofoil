#include "LZWDecompressor.h"

#include "CSInputBuffer.h"

LZWDecompressor::LZWDecompressor(int compressFlags)
	: input(nullptr)
	, compressFlags(compressFlags)
	, blockmode(false)
	, lzw(nullptr)
	, symbolcounter(0)
	, buffer(nullptr)
	, bufferend(nullptr)
{
}

LZWDecompressor::~LZWDecompressor()
{
	FreeLZW(lzw);
}


bool LZWDecompressor::Reset(CSInputBuffer *input, size_t compressedSize, size_t decompressedSize)
{
	this->input = input;
	blockmode = (compressFlags & 0x80) != 0;
	lzw = AllocLZW(1 << (compressFlags & 0x1f), blockmode ? 1 : 0);

	return true;
}

bool LZWDecompressor::ReadBytes(void *dest, size_t numBytes)
{
	uint8_t *nextByte = static_cast<uint8_t*>(dest);

	while (numBytes--)
	{
		if (buffer >= bufferend)
		{
			unsigned int symbol;
			for (;;)
			{
				bool atEOF;
				if (!CSInputAtEOF(input, atEOF))
					return false;

				if (atEOF)
					return false;

				if (!CSInputNextBitStringLE(input, LZWSuggestedSymbolSize(lzw), symbol))
					return false;

				symbolcounter++;
				if (symbol == 256 && blockmode)
				{
					// Skip garbage data after a clear. God damn, this is dumb.
					int symbolsize = LZWSuggestedSymbolSize(lzw);
					if (symbolcounter % 8)
					{
						if (!CSInputSkipBitsLE(input, symbolsize*(8 - symbolcounter % 8)))
							return false;
					}
					ClearLZWTable(lzw);
					symbolcounter = 0;
				}
				else
					break;
			}

			if (NextLZWSymbol(lzw, symbol) == LZWInvalidCodeError)
				return false;

			int n = LZWOutputToInternalBuffer(lzw);
			buffer = LZWInternalBuffer(lzw);
			bufferend = buffer + n;
		}

		*nextByte++ = *buffer++;
	}

	return true;
}
