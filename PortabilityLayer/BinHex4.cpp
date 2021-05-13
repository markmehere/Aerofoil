#include "BinHex4.h"
#include "GpIOStream.h"
#include "GpVector.h"

#include <string.h>
#include <assert.h>

// See: https://files.stairways.com/other/binhex-40-specs-info.txt
// Unfortunately, while the spec specifies that decoding is to be done high-to-low,
// it doesn't specify how the encoded 6-bit value is split.

#include "MacFileInfo.h"
#include "ByteUnpack.h"
#include "XModemCRC.h"
#include "MacFileMem.h"

namespace
{
	static bool IsEOL(char c)
	{
		return c == '\r' || c == '\n';
	}

	static bool IsWhitespaceChar(char c)
	{
		return c == '\r' || c == '\n' || c == ' ' || c == '\t';
	}

	uint16_t BinHexCRCNoPadding(const uint8_t *bytes, size_t size, int initialValue)
	{
		uint16_t crc = initialValue;
		for (size_t b = 0; b < size; b++)
		{
			uint8_t v = bytes[b];

			for (int i = 0; i < 8; i++)
			{
				int temp = (crc & 0x8000);
				crc = (crc << 1) | (v >> 7);

				if (temp)
					crc = crc ^ 0x1021;

				v = (v << 1) & 0xff;
			}
		}

		return static_cast<uint16_t>(crc);
	}

	uint16_t BinHexCRC(const uint8_t *bytes, size_t size)
	{
		const uint8_t zeroBytes[] = { 0, 0 };

		uint16_t crc = BinHexCRCNoPadding(bytes, size, 0);
		return BinHexCRCNoPadding(zeroBytes, 2, crc);
	}
}

namespace PortabilityLayer
{
	MacFileMem *BinHex4::LoadHQX(GpIOStream *stream, IGpAllocator *alloc)
	{
		const uint8_t errCodeChar = 64;

		uint8_t charMap[128];
		for (int i = 0; i < 128; i++)
			charMap[i] = errCodeChar;

		const char binHexCharacters[] = "!\"#$%&'()*+,-012345689@ABCDEFGHIJKLMNPQRSTUVXYZ[`abcdefhijklmpqr";

		for (int i = 0; i < 64; i++)
			charMap[binHexCharacters[i]] = static_cast<uint8_t>(i);

		const char expectedPrefix[] = "(This file must be converted with BinHex";
		const size_t prefixSizeChars = sizeof(expectedPrefix) - 1;
		char prefix[prefixSizeChars];

		if (stream->Read(prefix, prefixSizeChars) != prefixSizeChars)
			return nullptr;

		if (memcmp(prefix, expectedPrefix, prefixSizeChars))
			return nullptr;

		// Find end char
		for (;;)
		{
			char nextChar;
			if (!stream->Read(&nextChar, 1))
				return nullptr;

			if (IsEOL(nextChar))
				break;
		}

		// Find start colon
		for (;;)
		{
			char nextChar;
			if (!stream->Read(&nextChar, 1))
				return nullptr;

			if (IsWhitespaceChar(nextChar))
				continue;
			else if (nextChar == ':')
				break;
			else
				return nullptr;
		}

		GpVector<uint8_t> bytesAfter6To8(alloc);

		if (stream->IsSeekable())
		{
			GpUFilePos_t filePos = stream->Tell();
			if (stream->SeekEnd(0))
			{
				GpUFilePos_t endPos = stream->Tell();
				if (!stream->SeekStart(filePos))
					return nullptr;

				if (endPos > filePos && (endPos - filePos) < SIZE_MAX / 6)
				{
					if (!bytesAfter6To8.Reserve(static_cast<size_t>(endPos - filePos) * 6 / 8))
						return nullptr;
				}
			}
		}

		// Undo 8-to-6 coding
		const size_t bufferCapacity = 128;
		size_t bufferReadPos = 0;
		size_t bufferSize = 0;
		char buffer[bufferCapacity];

		bool isEOF = false;

		int decodedByte = 0;
		int decodedByteBitPos = 8;

		for (;;)
		{
			if (bufferReadPos == bufferSize)
			{
				const size_t numRead = stream->Read(buffer, bufferCapacity);
				if (numRead == 0)
					return nullptr;	// Missing terminator

				bufferSize = numRead;
				bufferReadPos = 0;
			}

			char nextChar = buffer[bufferReadPos++];

			if (nextChar == ':')
				break;

			if (IsWhitespaceChar(nextChar))
				continue;

			if (nextChar < 0 || nextChar > 127)
				return nullptr;

			uint8_t value6Bit = charMap[nextChar];
			if (value6Bit == errCodeChar)
				return nullptr;

			switch (decodedByteBitPos)
			{
			case 8:
				decodedByte = value6Bit << 2;
				decodedByteBitPos = 2;
				break;
			case 6:
				decodedByte |= value6Bit;
				if (!bytesAfter6To8.Append(decodedByte))
					return nullptr;
				decodedByte = 0;
				decodedByteBitPos = 8;
				break;
			case 4:
				decodedByte |= (value6Bit >> 2);
				if (!bytesAfter6To8.Append(decodedByte))
					return nullptr;
				decodedByte = (value6Bit << 6) & 0xff;
				decodedByteBitPos = 6;
				break;
			case 2:
				decodedByte |= (value6Bit >> 4);
				if (!bytesAfter6To8.Append(decodedByte))
					return nullptr;
				decodedByte = (value6Bit << 4) & 0xff;
				decodedByteBitPos = 4;
				break;
			default:
				return nullptr;
			}
		}

		const size_t bytesBeforeRLEDec = bytesAfter6To8.Count();
		size_t decodedDataSize = 0;
		for (size_t i = 0; i < bytesBeforeRLEDec; i++)
		{
			const uint8_t b = bytesAfter6To8[i];
			if (b == 0x90)
			{
				if (i == bytesBeforeRLEDec - 1)
					return nullptr;

				const uint8_t runLength = bytesAfter6To8[++i];

				if (runLength == 0)
					decodedDataSize++;	// 0x90 literal
				else
					decodedDataSize += runLength - 1;	// RLE, runs of length 1 are permitted
			}
			else
				decodedDataSize++;
		}

		GpVector<uint8_t> decodedBytes(alloc);
		if (!decodedBytes.Reserve(decodedDataSize))
			return nullptr;

		for (size_t i = 0; i < bytesBeforeRLEDec; i++)
		{
			const uint8_t b = bytesAfter6To8[i];

			if (b == 0x90)
			{
				const uint8_t runLength = bytesAfter6To8[++i];

				if (runLength == 0)
				{
					if (!decodedBytes.Append(0x90))
						return nullptr;
				}
				else
				{
					if (decodedBytes.Count() == 0)
						return nullptr;

					const uint8_t lastByte = decodedBytes[decodedBytes.Count() - 1];
					for (size_t r = 1; r < runLength; r++)
					{
						if (!decodedBytes.Append(lastByte))
							return nullptr;
					}
				}
			}
			else
			{
				if (!decodedBytes.Append(b))
					return nullptr;
			}
		}

		assert(decodedBytes.Count() == decodedDataSize);

		if (decodedBytes.Count() == 0)
			return nullptr;

		const uint8_t nameLength = decodedBytes[0];
		if (decodedBytes.Count() < 22 + nameLength || nameLength > 63)
			return nullptr;

		// Header format:
		// uint8_t nameLength
		// char name[nameLength]
		// char <null>
		// char fileType[4]
		// char fileCreator[4]
		// word flags
		// dword dataLength
		// dword resourceLength
		// word headerCRC

		const size_t headerStartLoc = 2 + nameLength;

		if (decodedBytes[nameLength + 1] != 0)
			return nullptr;

		MacFileInfo mfi;
		mfi.m_fileName.Set(nameLength, reinterpret_cast<const char*>(&decodedBytes[1]));
		memcpy(mfi.m_properties.m_fileType, &decodedBytes[headerStartLoc + 0], 4);
		memcpy(mfi.m_properties.m_fileCreator, &decodedBytes[headerStartLoc + 4], 4);
		mfi.m_properties.m_finderFlags = ByteUnpack::BigUInt16(&decodedBytes[headerStartLoc + 8]);
		mfi.m_dataForkSize = ByteUnpack::BigUInt32(&decodedBytes[headerStartLoc + 10]);
		mfi.m_resourceForkSize = ByteUnpack::BigUInt32(&decodedBytes[headerStartLoc + 14]);

		const size_t availableDataSize = decodedBytes.Count() - 26 - nameLength;	// +4 bytes for CRCs

		if (mfi.m_dataForkSize > availableDataSize || availableDataSize - mfi.m_dataForkSize < mfi.m_resourceForkSize)
			return nullptr;

		const uint16_t expectedHeaderCRC = ByteUnpack::BigUInt16(&decodedBytes[headerStartLoc + 18]);
		const uint16_t actualHeaderCRC = BinHexCRC(&decodedBytes[0], headerStartLoc + 18);

		if (expectedHeaderCRC != actualHeaderCRC)
			return nullptr;

		const size_t dataForkStart = headerStartLoc + 20;
		const size_t dataForkCRCLoc = dataForkStart + mfi.m_dataForkSize;
		const size_t resourceForkStart = dataForkCRCLoc + 2;
		const size_t resourceForkCRCLoc = resourceForkStart + mfi.m_resourceForkSize;

		const uint16_t expectedDataCRC = ByteUnpack::BigUInt16(&decodedBytes[dataForkCRCLoc]);
		const uint16_t expectedResCRC = ByteUnpack::BigUInt16(&decodedBytes[resourceForkCRCLoc]);

		if (expectedDataCRC != BinHexCRC(&decodedBytes[dataForkStart], mfi.m_dataForkSize))
			return nullptr;

		if (expectedResCRC != BinHexCRC(&decodedBytes[resourceForkStart], mfi.m_resourceForkSize))
			return nullptr;

		return MacFileMem::Create(alloc, &decodedBytes[dataForkStart], &decodedBytes[resourceForkStart], nullptr, mfi);
	}
}
