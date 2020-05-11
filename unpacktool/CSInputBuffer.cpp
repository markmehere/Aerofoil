#include "CSInputBuffer.h"
#include "IFileReader.h"

#include <stdlib.h>
#include <string.h>

// Allocation and management

CSInputBuffer *CSInputBufferAlloc(IFileReader *parent, int size)
{
	CSInputBuffer *self = static_cast<CSInputBuffer*>(malloc(sizeof(CSInputBuffer) + size));
	if (!self) return NULL;

	self->parent = parent;
	self->startoffs = parent->GetPosition();
	self->eof = false;

	self->buffer = (uint8_t *)&self[1];
	self->bufsize = size;
	self->bufbytes = 0;
	self->currbyte = 0;
	self->bits = 0;
	self->numbits = 0;

	return self;
}

CSInputBuffer *CSInputBufferAllocWithBuffer(const uint8_t *buffer, int length, IFileReader::FilePos_t startoffs)
{
	CSInputBuffer *self = static_cast<CSInputBuffer*>(malloc(sizeof(CSInputBuffer)));
	if (!self) return NULL;

	self->parent = NULL;
	self->startoffs = -startoffs;
	self->eof = true;

	self->buffer = (uint8_t *)buffer; // Since eof is set, the buffer won't be written to.
	self->bufsize = length;
	self->bufbytes = length;
	self->currbyte = 0;
	self->bits = 0;
	self->numbits = 0;

	return self;
}

CSInputBuffer *CSInputBufferAllocEmpty()
{
	CSInputBuffer *self = static_cast<CSInputBuffer*>(malloc(sizeof(CSInputBuffer)));
	if (!self) return NULL;

	self->parent = NULL;
	self->startoffs = 0;
	self->eof = true;

	self->buffer = NULL;
	self->bufsize = 0;
	self->bufbytes = 0;
	self->currbyte = 0;
	self->bits = 0;
	self->numbits = 0;

	return self;
}

void CSInputBufferFree(CSInputBuffer *self)
{
	free(self);
}

void CSInputSetMemoryBuffer(CSInputBuffer *self, uint8_t *buffer, int length, IFileReader::FilePos_t startoffs)
{
	self->eof = true;
	self->startoffs = -startoffs;
	self->buffer = buffer;
	self->bufsize = length;
	self->bufbytes = length;
	self->currbyte = 0;
	self->bits = 0;
	self->numbits = 0;
}





// Buffer and file positioning

void CSInputRestart(CSInputBuffer *self)
{
	CSInputSeekToFileOffset(self, self->startoffs);
}

void CSInputFlush(CSInputBuffer *self)
{
	self->currbyte = self->bufbytes = 0;
	self->bits = 0;
	self->numbits = 0;
}

void CSInputSynchronizeFileOffset(CSInputBuffer *self)
{
	CSInputSeekToFileOffset(self, CSInputFileOffset(self));
}

void CSInputSeekToFileOffset(CSInputBuffer *self, IFileReader::FilePos_t offset)
{
	self->parent->SeekStart(offset);
	self->eof = false;
	CSInputFlush(self);
}

void CSInputSeekToBufferOffset(CSInputBuffer *self, IFileReader::FilePos_t offset)
{
	CSInputSeekToFileOffset(self, offset + self->startoffs);
}

void CSInputSetStartOffset(CSInputBuffer *self, IFileReader::FilePos_t offset)
{
	self->startoffs = offset;
}

IFileReader::FilePos_t CSInputBufferOffset(CSInputBuffer *self)
{
	return CSInputFileOffset(self) - self->startoffs;
}

IFileReader::FilePos_t CSInputFileOffset(CSInputBuffer *self)
{
	if (self->parent) return self->parent->GetPosition() - self->bufbytes + self->currbyte;
	else return self->currbyte;
}

IFileReader::FilePos_t CSInputBufferBitOffset(CSInputBuffer *self)
{
	return CSInputBufferOffset(self) * 8 - (self->numbits & 7);
}




// Byte reading

void _CSInputFillBuffer(CSInputBuffer *self)
{
	int left = _CSInputBytesLeftInBuffer(self);

	if (left >= 0) memmove(self->buffer, self->buffer + self->currbyte, left);
	else
	{
		self->parent->SeekCurrent(-left);
		left = 0;
	}

	int actual = self->parent->Read(self->buffer + left, self->bufsize - left);
	if (actual == 0) self->eof = true;

	self->bufbytes = left + actual;
	self->currbyte = 0;
}




// Bitstream reading

// TODO: clean up and/or make faster
bool _CSInputFillBits(CSInputBuffer *self)
{
	_CSInputCheckAndFillBuffer(self);

	int numbytes = (32 - self->numbits) >> 3;
	int left = _CSInputBytesLeftInBuffer(self);
	if (numbytes > left) numbytes = left;

	int startoffset = self->numbits >> 3;
	//	int shift=24-self->numbits;

	//	for(int i=0;i<numbytes;i++)
	//	{
	//		self->bits|=_CSInputPeekByteWithoutEOF(self,i+startoffset)<<shift;
	//		shift-=8;
	//	}

	switch (numbytes)
	{
	case 4:
		self->bits =
			(_CSInputPeekByteWithoutEOF(self, startoffset) << 24) |
			(_CSInputPeekByteWithoutEOF(self, startoffset + 1) << 16) |
			(_CSInputPeekByteWithoutEOF(self, startoffset + 2) << 8) |
			_CSInputPeekByteWithoutEOF(self, startoffset + 3);
		break;
	case 3:
		self->bits |= (
			(_CSInputPeekByteWithoutEOF(self, startoffset) << 16) |
			(_CSInputPeekByteWithoutEOF(self, startoffset + 1) << 8) |
			(_CSInputPeekByteWithoutEOF(self, startoffset + 2) << 0)
			) << (8 - self->numbits);
		break;
	case 2:
		self->bits |= (
			(_CSInputPeekByteWithoutEOF(self, startoffset) << 8) |
			(_CSInputPeekByteWithoutEOF(self, startoffset + 1) << 0)
			) << (16 - self->numbits);
		break;
	case 1:
		self->bits |= _CSInputPeekByteWithoutEOF(self, startoffset) << (24 - self->numbits);
		break;
	}

	self->numbits += numbytes * 8;

	return true;
}

bool _CSInputFillBitsLE(CSInputBuffer *self)
{
	_CSInputCheckAndFillBuffer(self);

	int numbytes = (32 - self->numbits) >> 3;
	int left = _CSInputBytesLeftInBuffer(self);
	if (numbytes > left) numbytes = left;

	int startoffset = self->numbits >> 3;

	for (int i = 0; i < numbytes; i++)
	{
		self->bits |= _CSInputPeekByteWithoutEOF(self, i + startoffset) << self->numbits;
		self->numbits += 8;
	}

	return true;
}

bool CSInputNextBit(CSInputBuffer *self, unsigned int &bit)
{
	if (!CSInputPeekBitString(self, 1, bit))
		return false;
	return CSInputSkipPeekedBits(self, 1);
}

bool CSInputNextBitLE(CSInputBuffer *self, unsigned int &bit)
{
	if (!CSInputPeekBitStringLE(self, 1, bit))
		return false;
	return CSInputSkipPeekedBitsLE(self, 1);
}

bool CSInputNextBitString(CSInputBuffer *self, int numbits, unsigned int &bits)
{
	if (numbits == 0)
	{
		bits = 0;
		return true;
	}
	if (!CSInputPeekBitString(self, numbits, bits))
		return false;
	return CSInputSkipPeekedBits(self, numbits);
}

bool CSInputNextBitStringLE(CSInputBuffer *self, int numbits, unsigned int &bits)
{
	if (numbits == 0)
	{
		bits = 0;
		return true;
	}
	if (!CSInputPeekBitStringLE(self, numbits, bits))
		return false;
	if (!CSInputSkipPeekedBitsLE(self, numbits))
		return false;
	return true;
}

bool CSInputNextLongBitString(CSInputBuffer *self, int numbits, unsigned int &bits)
{
	if (numbits <= 25)
		return CSInputNextBitString(self, numbits, bits);
	else
	{
		int rest = numbits - 25;
		unsigned int readBits;
		if (!CSInputNextBitString(self, 25, readBits))
			return false;
		readBits <<= rest;
		unsigned int moreBits;
		if (!CSInputNextBitString(self, rest, moreBits))
			return false;
		bits = (readBits | moreBits);
		return true;
	}
}

bool CSInputNextLongBitStringLE(CSInputBuffer *self, int numbits, unsigned int &bits)
{
	if (numbits <= 25)
		return CSInputNextBitStringLE(self, numbits, bits);
	else
	{
		int rest = numbits - 25;
		unsigned int readBits;
		if (!CSInputNextBitStringLE(self, 25, readBits))
			return false;
		unsigned int moreBits;
		if (!CSInputNextBitStringLE(self, rest, moreBits))
			return false;
		bits = (readBits | (moreBits << 25));
		return true;
	}
}

bool CSInputSkipBits(CSInputBuffer *self, int numbits)
{
	if (numbits <= self->numbits)
		return CSInputSkipPeekedBits(self, numbits);
	else
	{
		int skipbits = numbits - (self->numbits & 7);
		CSInputSkipToByteBoundary(self);
		CSInputSkipBytes(self, skipbits >> 3);
		if (skipbits & 7)
		{
			unsigned int scratch;
			if (!CSInputNextBitString(self, skipbits & 7, scratch))
				return false;
		}

		return true;
	}
}

bool CSInputSkipBitsLE(CSInputBuffer *self, int numbits)
{
	if (numbits <= self->numbits)
		return CSInputSkipPeekedBitsLE(self, numbits);
	else
	{
		int skipbits = numbits - (self->numbits & 7);
		CSInputSkipToByteBoundary(self);
		CSInputSkipBytes(self, skipbits >> 3);
		if (skipbits & 7)
		{
			unsigned int scratch;
			if (!CSInputNextBitStringLE(self, skipbits & 7, scratch))
				return false;
		}

		return true;
	}
}



bool CSInputOnByteBoundary(CSInputBuffer *self)
{
	return (self->numbits & 7) == 0;
}

bool CSInputSkipToByteBoundary(CSInputBuffer *self)
{
	self->bits = 0;
	self->numbits = 0;

	return true;
}

bool CSInputSkipTo16BitBoundary(CSInputBuffer *self)
{
	if (!CSInputSkipToByteBoundary(self))
		return false;
	if (CSInputBufferOffset(self) & 1)
	{
		if (!CSInputSkipBytes(self, 1))
			return false;
	}

	return true;
}
