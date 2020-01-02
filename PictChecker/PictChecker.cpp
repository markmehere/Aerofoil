#define _CRT_SECURE_NO_WARNINGS

#include "MemReaderStream.h"
#include "MMHandleBlock.h"
#include "ResourceCompiledTypeList.h"
#include "ResourceFile.h"
#include "ResourceManager.h"
#include "CFileStream.h"
#include "QDPictDecoder.h"
#include "QDPictEmitContext.h"
#include "QDPictEmitScanlineParameters.h"
#include "QDPictHeader.h"
#include "QDPictOpcodes.h"
#include "RGBAColor.h"
#include "SharedTypes.h"
#include "stb_image_write.h"

#include <string>
#include <vector>
#include <algorithm>
#include <assert.h>

using namespace PortabilityLayer;

struct TempImage
{
	size_t m_width;
	size_t m_height;

	Rect m_rect;

	RGBAColor *m_contents;
};

void UnpackBits8(std::vector<uint8_t> &outVector, const std::vector<uint8_t> &inData)
{
	size_t count = inData.size();
	size_t offset = 0;
	while (offset < count)
	{
		int8_t headerByte;
		memcpy(&headerByte, &inData[offset++], 1);

		if (headerByte >= 0)
		{
			const size_t litCount = headerByte + 1;
			for (size_t i = 0; i < litCount; i++)
				outVector.push_back(inData[offset++]);
		}
		else if (headerByte < 0 && headerByte != -128)
		{
			const size_t repCount = (1 - headerByte);
			const uint8_t repeated = inData[offset++];
			for (size_t i = 0; i < repCount; i++)
				outVector.push_back(repeated);
		}
	}
}

void UnpackBits16(std::vector<uint16_t> &outVector, const std::vector<uint8_t> &inData)
{
	size_t count = inData.size();
	size_t offset = 0;
	while (offset < count)
	{
		int8_t headerByte;
		memcpy(&headerByte, &inData[offset++], 1);

		if (headerByte >= 0)
		{
			const size_t litCount = headerByte + 1;
			for (size_t i = 0; i < litCount; i++)
			{
				const uint8_t highByte = inData[offset++];
				const uint8_t lowByte = inData[offset++];
				const uint16_t combined = (highByte << 8) | lowByte;
				outVector.push_back(combined);
			}
		}
		else if (headerByte < 0 && headerByte != -128)
		{
			const size_t repCount = (1 - headerByte);

			const uint8_t highByte = inData[offset++];
			const uint8_t lowByte = inData[offset++];
			const uint16_t combined = (highByte << 8) | lowByte;

			for (size_t i = 0; i < repCount; i++)
				outVector.push_back(combined);
		}
	}
}

void BlitLineRGB8(const std::vector<uint8_t> &data, size_t first, RGBAColor *outScanline, size_t actualSize)
{
	for (size_t i = 0; i < actualSize; i++)
	{
		RGBAColor &color = outScanline[i];
		color.r = data[first + 0 * actualSize + i];
		color.g = data[first + 1 * actualSize + i];
		color.b = data[first + 2 * actualSize + i];
		color.a = 255;
	}
}

uint8_t FiveToEight(uint8_t v)
{
	return static_cast<uint8_t>((v << 3) | (v >> 2));
}

void BlitLine16(const std::vector<uint16_t> &data, size_t first, RGBAColor *outScanline, size_t actualSize)
{
	for (size_t i = 0; i < actualSize; i++)
	{
		RGBAColor &color = outScanline[i];
		const uint16_t packedInput = data[first + i];
		color.b = FiveToEight(packedInput & 0x1f);
		color.g = FiveToEight((packedInput >> 5) & 0x1f);
		color.r = FiveToEight((packedInput >> 10) & 0x1f);

		color.a = 255;
	}
}

void BlitLine1(const std::vector<uint8_t> &data, size_t first, RGBAColor *outScanline, size_t actualSize)
{
	for (size_t i = 0; i < actualSize; i++)
	{
		RGBAColor &color = outScanline[i];

		if ((data[first + i / 8] & (128 >> (i & 0x7))) == 0)
			color.r = color.g = color.b = 255;
		else
			color.r = color.g = color.b = 0;
		color.a = 255;
	}
}

void BlitLineIndexed(const std::vector<uint8_t> &data, size_t first, RGBAColor *outScanline, size_t actualSize, int componentSize, const RGBAColor *clut)
{
	if (componentSize == 1)
	{
		for (size_t i = 0; i < actualSize; i++)
		{
			const int bitShift = 7 - (i & 7);
			const int colorIndex = (data[first + i / 8] >> bitShift) & 0x1;
			outScanline[i] = clut[colorIndex];
		}
	}
	else if (componentSize == 2)
	{
		for (size_t i = 0; i < actualSize; i++)
		{
			const int bitShift = 6 - (2 * (i & 3));
			const int colorIndex = (data[first + i / 4] >> bitShift) & 0x3;
			outScanline[i] = clut[colorIndex];
		}
	}
	else if (componentSize == 4)
	{
		for (size_t i = 0; i < actualSize; i++)
		{
			const int bitShift = 4 - (4 * (i & 1));
			const int colorIndex = (data[first + i / 2] >> bitShift) & 0xf;
			outScanline[i] = clut[colorIndex];
		}
	}
	else if (componentSize == 8)
	{
		for (size_t i = 0; i < actualSize; i++)
			outScanline[i] = clut[data[first + i]];
	}
	else
	{
		assert(false);
	}
}

void DecodeClutItemChannel(uint8_t &outChannel, const uint8_t *inChannel)
{
	outChannel = inChannel[0];
}

void DecodeClutItem(RGBAColor &decoded, const BEColorTableItem &clutItem)
{
	DecodeClutItemChannel(decoded.r, clutItem.m_red);
	DecodeClutItemChannel(decoded.g, clutItem.m_green);
	DecodeClutItemChannel(decoded.b, clutItem.m_blue);
	decoded.a = 255;
}

void AuditPackBitsRectOld(MemReaderStream &stream, int pictVersion, bool isPackedFlag, bool hasRegion, bool isDirect, TempImage &image)
{
	uint16_t rowSizeBytes = 0;

	bool isPixMap = false;
	if (isDirect)
	{
		isPixMap = true;
		// Skip base address (4) + unknown (2)
		stream.SeekCurrent(4);

		stream.Read(&rowSizeBytes, sizeof(BEUInt16_t));
		ByteSwap::BigUInt16(rowSizeBytes);

		rowSizeBytes &= 0x7fff;
	}
	else
	{
		stream.Read(&rowSizeBytes, sizeof(BEUInt16_t));
		ByteSwap::BigUInt16(rowSizeBytes);

		if (pictVersion == 2)
		{
			if ((rowSizeBytes & 0x8000) != 0)
			{
				isPixMap = true;
				rowSizeBytes &= 0x7fff;
			}
			else
				printf("Actually a bitmap\n");
		}
	}

	if (!isPixMap)
	{
		printf("Bitmap?\n");
		BEBitMap bitMap;
		stream.Read(&bitMap, sizeof(BEBitMap));

		BERect srcRectBE;
		stream.Read(&srcRectBE, sizeof(BERect));

		Rect srcRect = srcRectBE.ToRect();

		BERect destRectBE;
		stream.Read(&destRectBE, sizeof(BERect));

		Rect destRect = destRectBE.ToRect();

		BEUInt16_t transferMode;
		stream.Read(&transferMode, 2);

		if (hasRegion)
		{
			BEUInt16_t regionSize;
			stream.Read(&regionSize, 2);

			stream.SeekCurrent(regionSize - 2);
		}

		assert(static_cast<size_t>(destRect.right) <= image.m_width);
		assert(static_cast<size_t>(destRect.left) <= image.m_width);
		assert(destRect.left < destRect.right);

		const int numCols = destRect.right - destRect.left;
		const int firstCol = destRect.left;
		const int firstRow = destRect.top;

		const int numLines = bitMap.m_bounds.bottom - bitMap.m_bounds.top;
		if (isPackedFlag && rowSizeBytes >= 8)
		{
			for (int i = 0; i < numLines; i++)
			{
				uint16_t lineByteCount;
				if (rowSizeBytes > 250)
				{
					stream.Read(&lineByteCount, 2);
					ByteSwap::BigUInt16(lineByteCount);
				}
				else
				{
					uint8_t lineByteCountSmall;
					stream.Read(&lineByteCountSmall, 1);
					lineByteCount = lineByteCountSmall;
				}

				std::vector<uint8_t> compressed;
				compressed.resize(lineByteCount);

				if (stream.Read(&compressed[0], lineByteCount) != lineByteCount)
				{
					assert(false);
				}

				std::vector<uint8_t> decompressed;
				UnpackBits8(decompressed, compressed);

				BlitLine1(decompressed, 0, image.m_contents + image.m_width * (firstRow + i) + firstCol, numCols);
			}
		}
		else
		{
			std::vector<uint8_t> decompressed;
			decompressed.resize(rowSizeBytes);

			for (int i = 0; i < numLines; i++)
			{
				if (stream.Read(&decompressed[0], rowSizeBytes) != rowSizeBytes)
				{
					assert(false);
				}

				BlitLine1(decompressed, 0, image.m_contents + image.m_width * (firstRow + i) + firstCol, numCols);
			}
		}
	}
	else
	{
		// Read RowBytes, if high bit is set, revert to bitmap
		// If rowBytes < 8 or pack type == 1, data is unpacked
		// If pack type == 2, pad byte of 32-bit data is dropped (direct pixels only, 32-bit images)
		// If pack type == 3, 16-bit RLE
		// If pack type == 4, 8-bit planar component RLE

		GP_STATIC_ASSERT(sizeof(BEPixMap) == 44);

		BEPixMap pixMap;
		stream.Read(&pixMap, sizeof(BEPixMap));

		printf("Pack type: %i\n", static_cast<int>(pixMap.m_packType));

		BEColorTableHeader clutHeader;
		BEColorTableItem clutItems[256];

		RGBAColor colors[256];
		for (int i = 0; i < 256; i++)
		{
			RGBAColor &color = colors[i];
			color.r = color.g = color.b = i;
			color.a = 255;
		}

		if (isDirect)
		{
		}
		else
		{
			stream.Read(&clutHeader, sizeof(BEColorTableHeader));

			assert(clutHeader.m_numItemsMinusOne <= 255);

			printf("CLUT items: %i\n", static_cast<int>(clutHeader.m_numItemsMinusOne));
			uint16_t numColors = clutHeader.m_numItemsMinusOne + 1;

			stream.Read(clutItems, sizeof(BEColorTableItem) * numColors);

			for (int i = 0; i < numColors; i++)
				DecodeClutItem(colors[i], clutItems[i]);
		}

		BERect srcRectBE;
		stream.Read(&srcRectBE, sizeof(BERect));

		BERect destRectBE;
		stream.Read(&destRectBE, sizeof(BERect));

		const Rect srcRect = srcRectBE.ToRect();
		const Rect destRect = destRectBE.ToRect();

		assert(static_cast<size_t>(destRect.right) <= image.m_width);
		assert(static_cast<size_t>(destRect.left) <= image.m_width);
		assert(destRect.left < destRect.right);

		const int numCols = destRect.right - destRect.left;
		const int firstCol = destRect.left;
		const int firstRow = destRect.top;

		BEUInt16_t transferMode;
		stream.Read(&transferMode, 2);

		//assert(srcRect.left == pixMap.m_bounds.left && srcRect.top == pixMap.m_bounds.top);
		//assert(srcRect.left == destRect.left && srcRect.top == destRect.top);


		assert(isPackedFlag || rowSizeBytes < 8 || pixMap.m_packType == 1);

		if (hasRegion)
		{
			BEUInt16_t regionSize;
			stream.Read(&regionSize, 2);

			stream.SeekCurrent(regionSize - 2);
		}

		int packType = pixMap.m_packType;
		if (isDirect && packType == 0)
		{
			switch (static_cast<int>(pixMap.m_componentSize))
			{
			case 16:
				packType = 3;
				break;
			case 32:
				packType = 4;
				break;
			default:
				break;
			}
		}

		if (isDirect)
		{
			if (packType == 4)
			{
				assert(pixMap.m_componentCount == 3);
				assert(pixMap.m_componentSize == 8);
			}
			else if (packType == 3)
			{
				assert(pixMap.m_componentCount == 3);
				assert(pixMap.m_componentSize == 5);
			}
			else
			{
				assert(false);
			}
		}

		int numLines = pixMap.m_bounds.bottom - pixMap.m_bounds.top;
		if (isDirect == false && (pixMap.m_packType == 1 || rowSizeBytes < 8))
		{
			std::vector<uint8_t> directLine;
			directLine.resize(rowSizeBytes);

			for (int i = 0; i < numLines; i++)
			{
				if (stream.Read(&directLine[0], rowSizeBytes) != rowSizeBytes)
				{
					assert(false);
				}

				if (pixMap.m_componentSize == 1)
				{
					BlitLine1(directLine, 0, image.m_contents + image.m_width * (firstRow + i) + firstCol, numCols);
				}
				else if (pixMap.m_componentSize == 8)
				{
					BlitLineIndexed(directLine, 0, image.m_contents + image.m_width * (firstRow + i) + firstCol, numCols, pixMap.m_componentSize, colors);
				}
				else
				{
					assert(false);
				}
			}
		}
		else if (isDirect && packType == 4)
		{
			assert(pixMap.m_componentCount == 3);
			assert(pixMap.m_componentSize == 8);

			for (int i = 0; i < numLines; i++)
			{
				uint16_t lineByteCount;
				if (rowSizeBytes > 250)
				{
					stream.Read(&lineByteCount, 2);
					ByteSwap::BigUInt16(lineByteCount);
				}
				else
				{
					uint8_t lineByteCountSmall;
					stream.Read(&lineByteCountSmall, 1);
					lineByteCount = lineByteCountSmall;
				}

				std::vector<uint8_t> compressedLine;
				compressedLine.resize(lineByteCount);

				if (stream.Read(&compressedLine[0], lineByteCount) != lineByteCount)
				{
					assert(false);
				}

				std::vector<uint8_t> decompressed;
				UnpackBits8(decompressed, compressedLine);

				BlitLineRGB8(decompressed, 0, image.m_contents + image.m_width * (firstRow + i) + firstCol, numCols);
			}
		}
		else if (isDirect && packType == 3)
		{
			for (int i = 0; i < numLines; i++)
			{
				uint16_t lineByteCount;
				if (rowSizeBytes > 250)
				{
					stream.Read(&lineByteCount, 2);
					ByteSwap::BigUInt16(lineByteCount);
				}
				else
				{
					uint8_t lineByteCountSmall;
					stream.Read(&lineByteCountSmall, 1);
					lineByteCount = lineByteCountSmall;
				}

				std::vector<uint8_t> compressedLine;
				compressedLine.resize(lineByteCount);

				if (stream.Read(&compressedLine[0], lineByteCount) != lineByteCount)
				{
					assert(false);
				}

				std::vector<uint16_t> decompressed;
				UnpackBits16(decompressed, compressedLine);

				BlitLine16(decompressed, 0, image.m_contents + image.m_width * (firstRow + i) + firstCol, numCols);
			}
		}
		else if (packType <= 1)
		{
			for (int i = 0; i < numLines; i++)
			{
				uint16_t lineByteCount;
				if (rowSizeBytes > 250)
				{
					stream.Read(&lineByteCount, 2);
					ByteSwap::BigUInt16(lineByteCount);
				}
				else
				{
					uint8_t lineByteCountSmall;
					stream.Read(&lineByteCountSmall, 1);
					lineByteCount = lineByteCountSmall;
				}

				std::vector<uint8_t> compressedLine;
				compressedLine.resize(lineByteCount);

				if (stream.Read(&compressedLine[0], lineByteCount) != lineByteCount)
				{
					assert(false);
				}

				std::vector<uint8_t> decompressed;
				UnpackBits8(decompressed, compressedLine);

				BlitLineIndexed(decompressed, 0, image.m_contents + image.m_width * (firstRow + i) + firstCol, numCols, pixMap.m_componentSize, colors);
			}
		}
		else
		{
			assert(false);
		}

		if ((stream.Tell() & 1) != 0)
			stream.SeekCurrent(1);
	}
}


void AuditPackBitsRect(MemReaderStream &stream, int pictVersion, bool isPackedFlag, bool hasRegion, bool isDirect, TempImage &image)
{
	if (false)
	{
		AuditPackBitsRectOld(stream, pictVersion, isPackedFlag, hasRegion, isDirect, image);
		return;
	}

	uint16_t rowSizeBytes = 0;

	bool isPixMap = false;
	if (isDirect)
	{
		isPixMap = true;
		// Skip base address
		stream.SeekCurrent(4);

		stream.Read(&rowSizeBytes, sizeof(BEUInt16_t));
		ByteSwap::BigUInt16(rowSizeBytes);

		rowSizeBytes &= 0x7fff;
	}
	else
	{
		stream.Read(&rowSizeBytes, sizeof(BEUInt16_t));
		ByteSwap::BigUInt16(rowSizeBytes);

		if (pictVersion == 2)
		{
			if ((rowSizeBytes & 0x8000) != 0)
			{
				isPixMap = true;
				rowSizeBytes &= 0x7fff;
			}
			else
				printf("Actually a bitmap\n");
		}
	}

	Rect srcRect;
	Rect destRect;
	uint16_t transferMode;

	BEColorTableHeader clutHeader;
	BEColorTableItem clutItems[256];

	RGBAColor colors[256];
	for (int i = 0; i < 256; i++)
	{
		RGBAColor &color = colors[i];
		color.r = color.g = color.b = i;
		color.a = 255;
	}

	BEPixMap pixMapBE;

	int packType = 0;

	if (!isPixMap)
	{
		BEBitMap bitMap;
		stream.Read(&bitMap, sizeof(BEBitMap));

		packType = (isPackedFlag && rowSizeBytes >= 8) ? 0 : 1;

		pixMapBE.m_bounds = bitMap.m_bounds;
		pixMapBE.m_version = 2;
		pixMapBE.m_packType = packType;
		pixMapBE.m_hRes = 72 << 16;
		pixMapBE.m_vRes = 72 << 16;
		pixMapBE.m_vRes = 72 << 16;
		pixMapBE.m_pixelType = 16;	// FIXME: Use direct instead
		pixMapBE.m_pixelSize = 1;
		pixMapBE.m_componentCount = 1;
		pixMapBE.m_componentSize = 1;
		pixMapBE.m_planeSizeBytes = 0;
		pixMapBE.m_clutHandle = 0;
		pixMapBE.m_unused = 0;

		BERect srcRectBE;
		stream.Read(&srcRectBE, sizeof(BERect));

		BERect destRectBE;
		stream.Read(&destRectBE, sizeof(BERect));

		srcRect = srcRectBE.ToRect();
		destRect = destRectBE.ToRect();

		stream.Read(&transferMode, 2);
		ByteSwap::BigUInt16(transferMode);

		if (hasRegion)
		{
			BEUInt16_t regionSize;
			stream.Read(&regionSize, 2);

			stream.SeekCurrent(regionSize - 2);
		}

		colors[0].r = colors[0].g = colors[0].b = 255;
		colors[0].a = 255;

		colors[1].r = colors[1].g = colors[1].b = 0;
		colors[1].a = 255;
	}
	else
	{
		// If rowBytes < 8 or pack type == 1, data is unpacked
		// If pack type == 2, pad byte of 32-bit data is dropped (direct pixels only, 32-bit images)
		// If pack type == 3, 16-bit RLE
		// If pack type == 4, 8-bit planar component RLE

		GP_STATIC_ASSERT(sizeof(BEPixMap) == 44);

		stream.Read(&pixMapBE, sizeof(BEPixMap));

		if (isDirect)
		{
		}
		else
		{
			stream.Read(&clutHeader, sizeof(BEColorTableHeader));

			assert(clutHeader.m_numItemsMinusOne <= 255);

			printf("CLUT items: %i\n", static_cast<int>(clutHeader.m_numItemsMinusOne));
			uint16_t numColors = clutHeader.m_numItemsMinusOne + 1;

			stream.Read(clutItems, sizeof(BEColorTableItem) * numColors);

			for (int i = 0; i < numColors; i++)
				DecodeClutItem(colors[i], clutItems[i]);
		}

		BERect srcRectBE;
		stream.Read(&srcRectBE, sizeof(BERect));

		BERect destRectBE;
		stream.Read(&destRectBE, sizeof(BERect));

		srcRect = srcRectBE.ToRect();
		destRect = destRectBE.ToRect();

		assert(destRect.left < destRect.right);
		assert(destRect.top < destRect.bottom);

		const int numCols = destRect.right - destRect.left;
		const int firstCol = destRect.left;
		const int firstRow = destRect.top;

		BEUInt16_t transferMode;
		stream.Read(&transferMode, 2);

		//assert(srcRect.left == pixMap.m_bounds.left && srcRect.top == pixMap.m_bounds.top);
		//assert(srcRect.left == destRect.left && srcRect.top == destRect.top);

		assert(srcRect.left == pixMapBE.m_bounds.left && srcRect.top == pixMapBE.m_bounds.top);

		assert(isPackedFlag || rowSizeBytes < 8 || pixMapBE.m_packType == 1);

		if (hasRegion)
		{
			BEUInt16_t regionSize;
			stream.Read(&regionSize, 2);

			stream.SeekCurrent(regionSize - 2);
		}

		packType = pixMapBE.m_packType;
		if (isDirect && packType == 0)
		{
			switch (static_cast<int>(pixMapBE.m_componentSize))
			{
			case 16:
				packType = 3;
				break;
			case 32:
				packType = 4;
				break;
			default:
				break;
			}
		}

		if (isDirect)
		{
			if (packType == 4)
			{
				assert(pixMapBE.m_componentCount == 3);
				assert(pixMapBE.m_componentSize == 8);
			}
			else if (packType == 3)
			{
				assert(pixMapBE.m_componentCount == 3);
				assert(pixMapBE.m_componentSize == 5);
			}
			else
			{
				assert(false);
			}
		}
	}


	assert(srcRect.right - srcRect.left == destRect.right - destRect.left);
	assert(srcRect.bottom - srcRect.top == destRect.bottom - destRect.top);

	assert(srcRect.right - srcRect.left <= pixMapBE.m_bounds.right - pixMapBE.m_bounds.left);
	assert(srcRect.bottom - srcRect.top <= pixMapBE.m_bounds.bottom - pixMapBE.m_bounds.top);

	bool noBlit = false;
	assert(destRect.left <= destRect.right);
	assert(destRect.top <= destRect.bottom);
	if (destRect.right <= image.m_rect.left)
		noBlit = true;
	if (destRect.left >= image.m_rect.right)
		noBlit = true;

	if (destRect.bottom <= image.m_rect.top)
		noBlit = true;
	if (destRect.top >= image.m_rect.bottom)
		noBlit = true;

	if (rowSizeBytes < 8)
		packType = 1;

	const Rect imageRect = image.m_rect;

	const int componentSize = pixMapBE.m_componentSize;

	const int numScanLines = pixMapBE.m_bounds.bottom - pixMapBE.m_bounds.top;
	const int scanLineWidth = pixMapBE.m_bounds.right - pixMapBE.m_bounds.left;
	const int scanLineLeft = destRect.left;
	const int scanLineTop = destRect.top;

	const int scanLineUsableWidth = std::min(scanLineWidth, static_cast<int>(destRect.right - destRect.left));
	
	if (!isDirect)
	{
		assert(packType == 0 || packType == 1);
	}

	std::vector<RGBAColor> scanline;
	scanline.resize(scanLineWidth);

	std::vector<uint8_t> decompressedScanline;
	if (packType == 1)
		decompressedScanline.resize(rowSizeBytes);

	for (int lineNum = 0; lineNum < numScanLines; lineNum++)
	{
		if (packType == 0 || packType > 2)
		{
			// 8-bit RLE
			uint16_t lineByteCount;
			if (rowSizeBytes > 250)
			{
				stream.Read(&lineByteCount, 2);
				ByteSwap::BigUInt16(lineByteCount);
			}
			else
			{
				uint8_t lineByteCountSmall;
				stream.Read(&lineByteCountSmall, 1);
				lineByteCount = lineByteCountSmall;
			}

			std::vector<uint8_t> compressedLine;
			compressedLine.resize(lineByteCount);

			if (stream.Read(&compressedLine[0], lineByteCount) != lineByteCount)
			{
				assert(false);
			}

			if (packType == 3)
			{
				std::vector<uint16_t> decompressed;
				UnpackBits16(decompressed, compressedLine);
				BlitLine16(decompressed, 0, &scanline[0], scanLineWidth);
			}
			else
			{
				std::vector<uint8_t> decompressed;
				UnpackBits8(decompressed, compressedLine);

				if (packType == 0)
					BlitLineIndexed(decompressed, 0, &scanline[0], scanLineWidth, componentSize, colors);
				else
				{
					assert(packType == 4);
					BlitLineRGB8(decompressed, 0, &scanline[0], scanLineWidth);
				}
			}
		}
		else if (packType == 1)
		{
			if (stream.Read(&decompressedScanline[0], rowSizeBytes) != rowSizeBytes)
			{
				assert(false);
			}

			if (componentSize == 1)
			{
				BlitLine1(decompressedScanline, 0, &scanline[0], scanLineWidth);
			}
			else if (componentSize == 8)
			{
				BlitLineIndexed(decompressedScanline, 0, &scanline[0], scanLineWidth, componentSize, colors);
			}
			else
			{
				assert(false);
			}
		}
		else
		{
			assert(false);
		}

		if (!noBlit)
		{
			const int yCoord = scanLineTop + lineNum;
			const int localYCoord = yCoord - image.m_rect.top;
			int localXCoordStart = scanLineLeft - image.m_rect.left;
			int localXCoordEnd = scanLineLeft + scanLineUsableWidth;

			if (localYCoord < 0 || yCoord >= image.m_rect.bottom)
				continue;

			size_t firstDestElement = localYCoord * image.m_width;
			size_t firstSrcElement = 0;
			size_t lastSrcElement = scanLineUsableWidth;

			if (localXCoordStart < 0)
			{
				firstSrcElement += static_cast<size_t>(-localXCoordStart);
				localXCoordStart = 0;
			}
			else if (localXCoordStart > 0)
			{
				firstDestElement += localXCoordStart;
			}

			if (localXCoordEnd > image.m_rect.right)
			{
				const size_t extra = localXCoordEnd - image.m_rect.right;
				lastSrcElement -= extra;
			}

			const size_t numElements = lastSrcElement - firstSrcElement;
			const size_t capacity = image.m_width * image.m_height;

			assert(firstDestElement <= capacity);
			assert(capacity - firstDestElement >= numElements);

			memcpy(image.m_contents + firstDestElement, &scanline[firstSrcElement], sizeof(RGBAColor) * numElements);
		}
	}

	/*
				BlitLineRGB8(decompressed, 0, image.m_contents + image.m_width * (firstRow + i) + firstCol, numCols);
			}
		}
		else if (isDirect && packType == 3)
		{
			for (int i = 0; i < numLines; i++)
			{
				uint16_t lineByteCount;
				if (rowSizeBytes > 250)
				{
					stream.Read(&lineByteCount, 2);
					ByteSwap::BigUInt16(lineByteCount);
				}
				else
				{
					uint8_t lineByteCountSmall;
					stream.Read(&lineByteCountSmall, 1);
					lineByteCount = lineByteCountSmall;
				}

				std::vector<uint8_t> compressedLine;
				compressedLine.resize(lineByteCount);

				if (stream.Read(&compressedLine[0], lineByteCount) != lineByteCount)
				{
					assert(false);
				}

				std::vector<uint16_t> decompressed;
				UnpackBits16(decompressed, compressedLine);

				BlitLine16(decompressed, 0, image.m_contents + image.m_width * (firstRow + i) + firstCol, numCols);
			}
		}
		else if (packType <= 1)
		{
			for (int i = 0; i < numLines; i++)
			{
				uint16_t lineByteCount;
				if (rowSizeBytes > 250)
				{
					stream.Read(&lineByteCount, 2);
					ByteSwap::BigUInt16(lineByteCount);
				}
				else
				{
					uint8_t lineByteCountSmall;
					stream.Read(&lineByteCountSmall, 1);
					lineByteCount = lineByteCountSmall;
				}

				std::vector<uint8_t> compressedLine;
				compressedLine.resize(lineByteCount);

				if (stream.Read(&compressedLine[0], lineByteCount) != lineByteCount)
				{
					assert(false);
				}

				std::vector<uint8_t> decompressed;
				UnpackBits8(decompressed, compressedLine);

				BlitLineIndexed(decompressed, 0, image.m_contents + image.m_width * (firstRow + i) + firstCol, numCols, pixMap.m_componentSize, colors);
			}
		}
		else
		{
			assert(false);
		}

	*/

	// Either undocumented behavior or non-compliant PICT resources, not sure
	if (isPixMap && (stream.Tell() & 1) != 0)
		stream.SeekCurrent(1);
}

class PictDumpEmitContext final : public QDPictEmitContext
{
public:
	bool SpecifyFrame(const Rect &rect) override
	{
		m_rect = rect;
		m_width = rect.right - rect.left;
		m_height = rect.bottom - rect.top;
		m_image.resize(m_width * m_height);
		return true;
	}

	Rect ConstrainRegion(const Rect &rect) const override
	{
		return rect.Intersect(m_rect);
	}

	void Start(QDPictBlitSourceType sourceType, const QDPictEmitScanlineParameters &params) override
	{
		m_blitType = sourceType;
		m_params = params;
		m_constraintRegionWidth = params.m_constrainedRegionRight - params.m_constrainedRegionLeft;
		m_constraintRegionStartIndex = params.m_constrainedRegionLeft - params.m_scanlineOriginX;
		m_constraintRegionEndIndex = params.m_constrainedRegionRight - params.m_scanlineOriginX;

		const size_t firstCol = params.m_constrainedRegionLeft - m_rect.left;
		const size_t firstRow = params.m_firstY - m_rect.top;

		m_outputIndexStart = firstRow * m_width + firstCol;
	}

	void BlitScanlineAndAdvance(const void *data) override
	{
		const int32_t crRight = m_params.m_constrainedRegionRight;
		const int32_t crLeft = m_params.m_constrainedRegionLeft;
		const size_t constraintRegionStartIndex = m_constraintRegionStartIndex;
		const uint8_t *dataBytes = static_cast<const uint8_t*>(data);
		const size_t outputIndexStart = m_outputIndexStart;
		const RGBAColor *palette = m_params.m_colors;
		const size_t planarSeparation = m_params.m_planarSeparation;

		static const RGBAColor bwColors[] =
		{
			{ 255, 255, 255, 255 },
			{ 0, 0, 0, 255 }
		};

		switch (m_blitType)
		{
		case QDPictBlitSourceType_Indexed1Bit:
			for (size_t i = 0; i < m_constraintRegionWidth; i++)
			{
				const size_t itemIndex = i + constraintRegionStartIndex;

				const int bitShift = 7 - (itemIndex & 7);
				const int colorIndex = (dataBytes[itemIndex / 8] >> bitShift) & 0x1;
				m_image[i + outputIndexStart] = palette[colorIndex];
			}
			break;
		case QDPictBlitSourceType_Indexed2Bit:
			for (size_t i = 0; i < m_constraintRegionWidth; i++)
			{
				const size_t itemIndex = i + constraintRegionStartIndex;

				const int bitShift = 6 - (2 * (itemIndex & 1));
				const int colorIndex = (dataBytes[itemIndex / 4] >> bitShift) & 0x3;
				m_image[i + outputIndexStart] = palette[colorIndex];
			}
			break;
		case QDPictBlitSourceType_Indexed4Bit:
			for (size_t i = 0; i < m_constraintRegionWidth; i++)
			{
				const size_t itemIndex = i + constraintRegionStartIndex;

				const int bitShift = 4 - (4 * (itemIndex & 1));
				const int colorIndex = (dataBytes[itemIndex / 2] >> bitShift) & 0xf;
				m_image[i + outputIndexStart] = palette[colorIndex];
			}
			break;
		case QDPictBlitSourceType_Indexed8Bit:
			for (size_t i = 0; i < m_constraintRegionWidth; i++)
			{
				const size_t itemIndex = i + constraintRegionStartIndex;
				const uint8_t colorIndex = dataBytes[itemIndex];
				m_image[i + outputIndexStart] = palette[colorIndex];
			}
			break;
		case QDPictBlitSourceType_1Bit:
			for (size_t i = 0; i < m_constraintRegionWidth; i++)
			{
				const size_t itemIndex = i + constraintRegionStartIndex;

				const int bitShift = 7 - (itemIndex & 7);
				const int colorIndex = (dataBytes[itemIndex / 8] >> bitShift) & 0x1;
				m_image[i + outputIndexStart] = bwColors[colorIndex];
			}
			break;
		case QDPictBlitSourceType_RGB15:
			for (size_t i = 0; i < m_constraintRegionWidth; i++)
			{
				const size_t itemIndex = i + constraintRegionStartIndex;

				const uint16_t item = *reinterpret_cast<const uint16_t*>(dataBytes + itemIndex * 2);
				RGBAColor &outputItem = m_image[i + outputIndexStart];

				outputItem.b = FiveToEight(item & 0x1f);
				outputItem.g = FiveToEight((item >> 5) & 0x1f);
				outputItem.r = FiveToEight((item >> 10) & 0x1f);
				outputItem.a = 255;
			}
			break;
		case QDPictBlitSourceType_RGB24_Multiplane:
			for (size_t i = 0; i < m_constraintRegionWidth; i++)
			{
				const size_t itemIndex = i + constraintRegionStartIndex;

				RGBAColor &outputItem = m_image[i + outputIndexStart];

				outputItem.r = dataBytes[itemIndex];
				outputItem.g = dataBytes[itemIndex + planarSeparation];
				outputItem.b = dataBytes[itemIndex + planarSeparation * 2];
				outputItem.a = 255;
			}
			break;
		default:
			assert(false);
		}

		m_outputIndexStart += m_width;
	}

	bool AllocTempBuffers(uint8_t *&buffer1, size_t buffer1Size, uint8_t *&buffer2, size_t buffer2Size) override
	{
		m_buffer1.resize(buffer1Size);
		m_buffer2.resize(buffer2Size);

		buffer1 = &m_buffer1[0];
		buffer2 = &m_buffer2[0];

		return true;
	}

	void DumpImageToFile(const char *path)
	{
		stbi_write_png(path, static_cast<int>(m_width), static_cast<int>(m_height), 4, &m_image[0], static_cast<int>(m_width * 4));
	}

private:
	Rect m_rect;
	int32_t m_originX;
	int32_t m_originY;

	size_t m_width;
	size_t m_height;
	size_t m_constraintRegionWidth;
	size_t m_constraintRegionStartIndex;
	size_t m_constraintRegionEndIndex;
	size_t m_outputIndexStart;
	QDPictBlitSourceType m_blitType;
	QDPictEmitScanlineParameters m_params;

	std::vector<uint8_t> m_buffer1;
	std::vector<uint8_t> m_buffer2;

	std::vector<RGBAColor> m_image;
};

void AuditPictOps2(const uint8_t *data, size_t sz, const char *dumpPath)
{
	MemReaderStream stream(data, sz);

	QDPictDecoder decoder;
	PictDumpEmitContext emitContext;

	decoder.DecodePict(&stream, &emitContext);

	emitContext.DumpImageToFile(dumpPath);
}

void AuditPictOps(const uint8_t *data, size_t sz, const char *dumpPath)
{
	MemReaderStream stream(data, sz);

	QDPictHeader header;

	header.Load(&stream);

	TempImage image;
	image.m_width = header.GetFrame().right - header.GetFrame().left;
	image.m_height = header.GetFrame().bottom - header.GetFrame().top;
	image.m_rect = header.GetFrame();
	image.m_contents = new RGBAColor[image.m_width * image.m_height];

	BERect scratchBERect;
	Rect scratchRect;
	BEUInt16_t scratchUInt16;
	BEUInt32_t scratchUInt32;

	printf("Decode\n");

	uint8_t scratchBytes[64];
	while (stream.Read(scratchBytes, header.GetVersion()) == header.GetVersion())
	{
		bool finished = false;

		uint16_t opcode = scratchBytes[0];
		if (header.GetVersion() == 2)
			opcode = (opcode << 8) | scratchBytes[1];

		printf("Opcode: %x\n", opcode);

		switch (opcode)
		{
		case QDOpcodes::kNoop:
			break;
		case QDOpcodes::kClipRegion:
			if (stream.Read(scratchBytes, 10) != 10 || scratchBytes[0] != 0 || scratchBytes[1] != 10)
				return;	// Unknown format region

			memcpy(&scratchBERect, scratchBytes + 2, 8);
			scratchRect = scratchBERect.ToRect();

			break;
		case QDOpcodes::kShortComment:
			stream.Read(scratchBytes, 2);
			break;
		case QDOpcodes::kLongComment:
			stream.Read(scratchBytes, 2);
			stream.Read(&scratchUInt16, 2);
			stream.SeekCurrent(scratchUInt16);
			break;
		case QDOpcodes::kBitsRect:
			AuditPackBitsRect(stream, header.GetVersion(), false, false, false, image);
			break;
		case QDOpcodes::kPackBitsRect:
			AuditPackBitsRect(stream, header.GetVersion(), true, false, false, image);
			break;
		case QDOpcodes::kPackBitsRgn:
			AuditPackBitsRect(stream, header.GetVersion(), true, true, false, image);
			break;
		case QDOpcodes::kDirectBitsRect:
			AuditPackBitsRect(stream, header.GetVersion(), true, false, true, image);
			break;
		case QDOpcodes::kDirectBitsRgn:
			AuditPackBitsRect(stream, header.GetVersion(), true, true, true, image);
			break;
		case QDOpcodes::kDefaultHilite:
			break;
		case QDOpcodes::kOpColor:
			stream.SeekCurrent(6);
			break;
		//case QDOpcodes::kFrameRect:
		//	stream.SeekCurrent(8);
		//	break;
		case QDOpcodes::kEndOfPicture:
			finished = true;
			break;
		default:
			assert(false);
		}

		if (finished)
			break;
	}

	stbi_write_png(dumpPath, image.m_width, image.m_height, 4, image.m_contents, image.m_width * 4);
	delete[] image.m_contents;
}

int main(int argc, const char **argv)
{
	const char *files[] = 
	{
		"Teddy World", "CD Demo House", "The Asylum Pro", "ApplicationResources", "California or Bust!", "Sampler", "Art Museum", "Castle o' the Air",
		"Davis Station", "Demo House", "Empty House", "Fun House", "Grand Prix", "ImagineHouse PRO II",
		"In The Mirror", "Land of Illusion", "Leviathan", "Metropolis", "Nemo's Market",
		"Rainbow's End", "Slumberland", "SpacePods", "Titanic"
	};

	for (const char *filename : files)
	{
		std::string fnameStr = filename;
		std::string filePath = (fnameStr == "ApplicationResources") ? "D:\\Source Code\\GlidePort\\Packaged\\" : "D:\\Source Code\\GlidePort\\Packaged\\Houses\\";
		filePath += filename;
		filePath += ".gpr";

		FILE *f = fopen(filePath.c_str(), "rb");
		CFileStream stream(f);

		ResourceFile *resFile = new ResourceFile();
		if (!resFile->Load(&stream))
		{
			assert(false);
		}
		stream.Close();

		const ResourceCompiledTypeList *typeList = resFile->GetResourceTypeList('PICT');
		if (!typeList)
			continue;

		const size_t numRefs = typeList->m_numRefs;
		for (size_t i = 0; i < numRefs; i++)
		{
			const MMHandleBlock *hBlock = resFile->GetResource('PICT', typeList->m_firstRef[i].m_resID, true);
			const void *pictData = hBlock->m_contents;

			std::string dumpPath = "D:\\Source Code\\GlidePort\\DebugData\\PictDump\\";

			dumpPath += filename;
			
			char tempPath[128];
			sprintf(tempPath, "%i", static_cast<int>(typeList->m_firstRef[i].m_resID));

			dumpPath += " ";
			dumpPath += tempPath;
			dumpPath += ".png";

			if (typeList->m_firstRef[i].m_resID == 10024)
			{
				int n = 0;
			}

			AuditPictOps2(static_cast<const uint8_t*>(pictData), hBlock->m_size, dumpPath.c_str());
		}
	}
	
	return 0;
}
