#include "CoreDefs.h"
#include "QDPictDecoder.h"
#include "QDPictEmitContext.h"
#include "QDPictHeader.h"
#include "QDPictOpcodes.h"
#include "QDPictEmitScanlineParameters.h"
#include "IOStream.h"
#include "RGBAColor.h"
#include "Vec2i.h"

#include <vector>
#include <assert.h>

namespace
{
	static void DecodeClutItemChannel(uint8_t &outChannel, const uint8_t *color16)
	{
		const int colorHigh = color16[0];
		const int colorLow = color16[1];

		const int lowDelta = colorLow - colorHigh;
		if (lowDelta < -128)
			outChannel = static_cast<uint8_t>(colorHigh - 1);
		else if (lowDelta > 128)
			outChannel = static_cast<uint8_t>(colorHigh + 1);
		outChannel = static_cast<uint8_t>(colorHigh);
	}

	static void DecodeClutItem(PortabilityLayer::RGBAColor &decoded, const BEColorTableItem &clutItem)
	{
		DecodeClutItemChannel(decoded.r, clutItem.m_red);
		DecodeClutItemChannel(decoded.g, clutItem.m_green);
		DecodeClutItemChannel(decoded.b, clutItem.m_blue);
		decoded.a = 255;
	}
}

namespace PortabilityLayer
{
	QDPictDecoder::QDPictDecoder()
		: m_stream(nullptr)
	{
	}

	bool QDPictDecoder::DecodePict(IOStream *stream, QDPictEmitContext *emitContext)
	{
		QDPictHeader header;

		if (!header.Load(stream))
			return false;

		emitContext->SpecifyFrame(header.GetFrame());

		const Rect constrainedFrame = emitContext->ConstrainRegion(header.GetFrame());
		Rect activeFrame = constrainedFrame;

		BERect scratchBERect;
		Rect scratchRect;
		BEUInt16_t scratchUInt16;
		BEUInt32_t scratchUInt32;

		const int pictVersion = header.GetVersion();
		if (pictVersion != 1 && pictVersion != 2)
			return false;

		const size_t opcodeSize = (pictVersion == 1) ? 1 : 2;

		uint8_t scratchBytes[64];

		for (;;)
		{
			if (stream->Read(scratchBytes, opcodeSize) != opcodeSize)
				return false;

			bool finished = false;

			uint16_t opcode = scratchBytes[0];
			if (header.GetVersion() == 2)
				opcode = (opcode << 8) | scratchBytes[1];

			int rasterOpErrorCode = 0;

			switch (opcode)
			{
			case QDOpcodes::kNoop:
				break;
			case QDOpcodes::kClipRegion:
				if (stream->Read(scratchBytes, 10) != 10 || scratchBytes[0] != 0 || scratchBytes[1] != 10)
					return false;	// Unknown format region

				PL_STATIC_ASSERT(sizeof(scratchBERect) == 8);

				memcpy(&scratchBERect, scratchBytes + 2, 8);
				scratchRect = scratchBERect.ToRect();

				if (!scratchRect.IsValid())
					return false;

				break;
			case QDOpcodes::kShortComment:
				if (!stream->SeekCurrent(2))
					return false;
				break;
			case QDOpcodes::kLongComment:
				{
					if (stream->Read(scratchBytes, 4) != 4)
						return false;

					const uint16_t commentKind = (scratchBytes[0] << 8) | scratchBytes[1];
					const uint16_t commentSize = (scratchBytes[2] << 8) | scratchBytes[3];

					if (!stream->SeekCurrent(commentSize))
						return false;
				}
				break;
			case QDOpcodes::kBitsRect:
				rasterOpErrorCode = ProcessRasterOp(stream, header.GetVersion(), false, false, false, activeFrame, emitContext);
				if (rasterOpErrorCode)
					return false;
				break;
			case QDOpcodes::kPackBitsRect:
				rasterOpErrorCode = ProcessRasterOp(stream, header.GetVersion(), true, false, false, activeFrame, emitContext);
				if (rasterOpErrorCode)
					return false;
				break;
			case QDOpcodes::kPackBitsRgn:
				rasterOpErrorCode = ProcessRasterOp(stream, header.GetVersion(), true, true, false, activeFrame, emitContext);
				if (rasterOpErrorCode)
					return false;
				break;
			case QDOpcodes::kDirectBitsRect:
				rasterOpErrorCode = ProcessRasterOp(stream, header.GetVersion(), true, false, true, activeFrame, emitContext);
				if (rasterOpErrorCode)
					return false;
				break;
			case QDOpcodes::kDirectBitsRgn:
				rasterOpErrorCode = ProcessRasterOp(stream, header.GetVersion(), true, true, true, activeFrame, emitContext);
				if (rasterOpErrorCode)
					return false;
				break;
			case QDOpcodes::kDefaultHilite:
				break;
			case QDOpcodes::kOpColor:
				if (!stream->SeekCurrent(6))
					return false;
				break;
			case QDOpcodes::kEndOfPicture:
				finished = true;
				break;
			default:
				// Unknown opcode
				return false;
			}

			if (finished)
				return true;
		}
	}

	int QDPictDecoder::ProcessRasterOp(IOStream *stream, int pictVersion, bool isPackedFlag, bool hasRegion, bool isDirect, const Rect &constraintRect, QDPictEmitContext *context)
	{
		uint16_t rowSizeBytes = 0;

		bool isPixMap = false;
		if (isDirect)
		{
			isPixMap = true;
			// Skip base address
			if (!stream->SeekCurrent(4))
				return 1;

			if (stream->Read(&rowSizeBytes, sizeof(uint16_t)) != sizeof(uint16_t))
				return 2;

			ByteSwap::BigUInt16(rowSizeBytes);

			rowSizeBytes &= 0x7fff;
		}
		else
		{
			if (stream->Read(&rowSizeBytes, sizeof(uint16_t)) != sizeof(uint16_t))
				return 3;

			ByteSwap::BigUInt16(rowSizeBytes);

			if (pictVersion == 2)
			{
				if ((rowSizeBytes & 0x8000) != 0)
				{
					isPixMap = true;
					rowSizeBytes &= 0x7fff;
				}
				// ... else this is a bitmap
			}
		}

		Rect srcRect;
		Rect destRect;
		uint16_t transferMode;

		BEColorTableHeader clutHeader;
		BEColorTableItem clutItems[256];

		BEPixMap pixMapBE;

		int packType = 0;

		RGBAColor colors[256];
		size_t numColors = 0;

		if (!isPixMap)
		{
			BEBitMap bitMap;
			if (stream->Read(&bitMap, sizeof(BEBitMap)) != sizeof(BEBitMap))
				return 4;

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
			if (stream->Read(&srcRectBE, sizeof(BERect)) != sizeof(BERect))
				return 5;

			BERect destRectBE;
			if (stream->Read(&destRectBE, sizeof(BERect)) != sizeof(BERect))
				return 6;

			srcRect = srcRectBE.ToRect();
			destRect = destRectBE.ToRect();

			if (!srcRect.IsValid())
				return 7;

			if (!destRect.IsValid())
				return 8;

			if (stream->Read(&transferMode, 2) != 2)
				return 9;

			ByteSwap::BigUInt16(transferMode);

			if (hasRegion)
			{
				BEUInt16_t regionSize;
				if (stream->Read(&regionSize, 2) != 2)
					return 10;

				if (regionSize < 2)
					return 11;

				if (!stream->SeekCurrent(regionSize - 2))
					return 12;
			}

			colors[0].r = colors[0].g = colors[0].b = 255;
			colors[0].a = 255;

			colors[1].r = colors[1].g = colors[1].b = 0;
			colors[1].a = 255;

			numColors = 0;
		}
		else
		{
			// If rowBytes < 8 or pack type == 1, data is unpacked
			// If pack type == 2, pad byte of 32-bit data is dropped (direct pixels only, 32-bit images)
			// If pack type == 3, 16-bit RLE
			// If pack type == 4, 8-bit planar component RLE

			PL_STATIC_ASSERT(sizeof(BEPixMap) == 44);

			if (stream->Read(&pixMapBE, sizeof(BEPixMap)) != sizeof(BEPixMap))
				return 13;

			if (isDirect)
			{
			}
			else
			{
				if (stream->Read(&clutHeader, sizeof(BEColorTableHeader)) != sizeof(BEColorTableHeader))
					return 14;

				const uint16_t numItemsMinusOne = clutHeader.m_numItemsMinusOne;
				if (numItemsMinusOne > 255)
					return 15;

				numColors = clutHeader.m_numItemsMinusOne + 1;

				if (stream->Read(clutItems, sizeof(BEColorTableItem) * numColors) != sizeof(BEColorTableItem) * numColors)
					return 16;

				for (size_t i = 0; i < numColors; i++)
					DecodeClutItem(colors[i], clutItems[i]);
			}

			BERect srcRectBE;
			if (stream->Read(&srcRectBE, sizeof(BERect)) != sizeof(BERect))
				return 17;

			BERect destRectBE;
			if (stream->Read(&destRectBE, sizeof(BERect)) != sizeof(BERect))
				return 18;

			srcRect = srcRectBE.ToRect();
			destRect = destRectBE.ToRect();

			if (!srcRect.IsValid() || !destRect.IsValid())
				return 19;

			if (stream->Read(&transferMode, 2) != 2)
				return 20;

			ByteSwap::BigUInt16(transferMode);

			if (!isPackedFlag && rowSizeBytes >= 8 && pixMapBE.m_packType != 1)
				return 21;

			if (hasRegion)
			{
				uint16_t regionSize;
				if (stream->Read(&regionSize, 2) != 2)
					return 22;

				ByteSwap::BigUInt16(regionSize);

				if (regionSize < 2)
					return 23;

				if (!stream->SeekCurrent(regionSize - 2))
					return 24;
			}

			const unsigned int pixelSize = pixMapBE.m_pixelSize;
			const unsigned int componentCount = pixMapBE.m_componentCount;
			const unsigned int componentSize = pixMapBE.m_componentSize;

			packType = pixMapBE.m_packType;
			if (isDirect)
			{
				if (packType == 0)
				{
					switch (pixelSize)
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

				if (packType == 4)
				{
					if (pixMapBE.m_componentCount != 3)
						return 25;

					if (pixMapBE.m_componentSize != 8)
						return 26;

					if (pixelSize != 32)
						return 27;
				}
				else if (packType == 3)
				{
					if (componentCount != 3)
						return 28;
					if (componentSize != 5)
						return 29;

					if (pixelSize != 16)
						return 30;
				}
				else
				{
					switch (pixelSize)
					{
					case 32:
						if (componentCount != 3 || componentSize != 8)
							return 31;
						break;
					case 16:
						if (componentCount != 3 || componentSize != 5)
							return 32;
						break;
					case 8:
						if (componentCount != 1 || componentSize != 8)
							return 33;
						break;
					default:
						return 34;
					}
				}
			}
			else
			{
				switch (pixMapBE.m_componentSize)
				{
				case 1:
				case 2:
				case 4:
				case 8:
					break;
				default:
					return 35;
				}

				if (componentSize != pixelSize)
					return 36;
			}

			if (packType > 4)
				return 37;
		}

		const int componentSize = pixMapBE.m_componentSize;

		// We only support rect moves that are the same size
		if (srcRect.right - srcRect.left != destRect.right - destRect.left)
			return 38;

		if (srcRect.bottom - srcRect.top != destRect.bottom - destRect.top)
			return 39;

		const Rect pixMapBounds = pixMapBE.m_bounds.ToRect();
		if (!pixMapBounds.IsValid())
			return 40;

		if (srcRect.left < pixMapBounds.left || srcRect.right > pixMapBounds.right || srcRect.top < pixMapBounds.top || srcRect.bottom > pixMapBounds.bottom)
			return 41;

		const Vec2i pixMapOriginRelativeToSrcRect = Vec2i(pixMapBounds.left, pixMapBounds.top) - Vec2i(srcRect.left, srcRect.top);
		const Vec2i pixMapOriginRelativeToDestRect = pixMapOriginRelativeToSrcRect;

		const Vec2i pixMapOrigin = pixMapOriginRelativeToDestRect + Vec2i(destRect.left, destRect.top);
		const Vec2i pixMapBottomRight = pixMapOrigin + Vec2i(pixMapBounds.right, pixMapBounds.bottom) - Vec2i(pixMapBounds.left, pixMapBounds.top);

		const Rect constrainedDestRect = constraintRect.Intersect(destRect).MakeValid();

		bool skipAll = false;
		if (!constrainedDestRect.IsValid())
			skipAll = true;

		if (rowSizeBytes < 8)
			packType = 1;

		if (!isDirect)
		{
			if (packType != 0 && packType != 1)
				return 42;
		}

		// NOT CURRENTLY SUPPORTED
		if (packType == 2)
			return 43;

		size_t decompressedRowSize = rowSizeBytes;

		const size_t minimumRowSize = (static_cast<size_t>(pixMapBounds.right - pixMapBounds.left) * static_cast<uint16_t>(pixMapBE.m_pixelSize) + 7) / 8;
		if (decompressedRowSize < minimumRowSize)
			return 56;

		if (packType == 4)
		{
			if ((rowSizeBytes & 3) != 0)
				return 55;

			// Fudge decompressed row size in planar RGB case
			decompressedRowSize = (rowSizeBytes / 4) * 3;
		}

		// Max compressed size is 4
		const size_t maxCompressedSize = decompressedRowSize + (decompressedRowSize / 128) + 4;

		uint8_t *decompressedScanlineBuffer = nullptr;
		uint8_t *compressedScanlineBuffer = nullptr;

		if (!context->AllocTempBuffers(decompressedScanlineBuffer, decompressedRowSize, compressedScanlineBuffer, maxCompressedSize))
			return 57;

		bool started = false;

		QDPictEmitScanlineParameters params;
		params.m_scanlineOriginX = pixMapOrigin.m_x;
		params.m_firstY = constrainedDestRect.top;
		params.m_constrainedRegionLeft = constrainedDestRect.left;
		params.m_constrainedRegionRight = constrainedDestRect.right;
		params.m_colors = colors;
		params.m_numColors = numColors;
		params.m_planarSeparation = pixMapBottomRight.m_x - pixMapOrigin.m_x;

		for (int32_t y = pixMapOrigin.m_y; y < pixMapBottomRight.m_y; y++)
		{
			bool isLineValid = !skipAll;
			if (y < constrainedDestRect.top || y >= constrainedDestRect.bottom)
				isLineValid = false;

			if (packType == 0 || packType > 2)
			{
				// RLE
				uint16_t lineByteCount;
				if (rowSizeBytes > 250)
				{
					if (stream->Read(&lineByteCount, 2) != 2)
						return 44;
					ByteSwap::BigUInt16(lineByteCount);
				}
				else
				{
					uint8_t lineByteCountSmall;
					if (stream->Read(&lineByteCountSmall, 1) != 1)
						return 45;
					lineByteCount = lineByteCountSmall;
				}

				if (!isLineValid)
				{
					if (!stream->SeekCurrent(lineByteCount))
						return 46;
					continue;
				}

				if (lineByteCount > maxCompressedSize)
					return 58;

				if (stream->Read(compressedScanlineBuffer, lineByteCount) != lineByteCount)
					return 47;

				if (packType == 3)
				{
					// 16-bit RLE
					if (!UnpackBits16(decompressedScanlineBuffer, decompressedRowSize, compressedScanlineBuffer, lineByteCount))
						return 48;

					if (!started)
					{
						context->Start(QDPictBlitSourceType_RGB15, params);
						started = true;
					}

					context->BlitScanlineAndAdvance(decompressedScanlineBuffer);
				}
				else
				{
					// 8-bit RLE
					if (!UnpackBits8(decompressedScanlineBuffer, decompressedRowSize, compressedScanlineBuffer, lineByteCount))
						return 49;

					if (!started)
					{
						if (packType == 0)
						{
							switch (componentSize)
							{
							case 1:
								context->Start(isPixMap ? QDPictBlitSourceType_Indexed1Bit : QDPictBlitSourceType_1Bit, params);
								break;
							case 2:
								context->Start(QDPictBlitSourceType_Indexed2Bit, params);
								break;
							case 4:
								context->Start(QDPictBlitSourceType_Indexed4Bit, params);
								break;
							case 8:
								context->Start(QDPictBlitSourceType_Indexed8Bit, params);
								break;
							default:
								return 50;	// ???
							}
						}
						else
						{
							assert(packType == 4);
							context->Start(QDPictBlitSourceType_RGB24_Multiplane, params);
						}
						started = true;
					}

					context->BlitScanlineAndAdvance(decompressedScanlineBuffer);
				}
			}
			else if (packType == 1)
			{
				if (stream->Read(decompressedScanlineBuffer, rowSizeBytes) != rowSizeBytes)
					return 51;

				if (!started)
				{
					switch (componentSize)
					{
					case 1:
						context->Start(isPixMap ? QDPictBlitSourceType_Indexed1Bit : QDPictBlitSourceType_1Bit, params);
						break;
					case 2:
						context->Start(QDPictBlitSourceType_Indexed2Bit, params);
						break;
					case 4:
						context->Start(QDPictBlitSourceType_Indexed4Bit, params);
						break;
					case 8:
						context->Start(QDPictBlitSourceType_Indexed8Bit, params);
						break;
					default:
						return 52;	// ???
					}

					started = true;
				}

				context->BlitScanlineAndAdvance(decompressedScanlineBuffer);
			}
			else
				return 53;
		}

		// Either undocumented behavior or non-compliant PICT resources, not sure
		if (isPixMap && (stream->Tell() & 1) != 0)
		{
			if (!stream->SeekCurrent(1))
				return 54;
		}

		return 0;
	}

	bool QDPictDecoder::UnpackBits8(uint8_t *dest, size_t destSize, const uint8_t *src, size_t srcSize)
	{
		while (srcSize > 0)
		{
			int8_t headerByte = *reinterpret_cast<const int8_t*>(src);
			src++;
			srcSize--;

			if (headerByte >= 0)
			{
				const size_t litCount = headerByte + 1;
				if (destSize < litCount || srcSize < litCount)
					return false;

				memcpy(dest, src, litCount);

				src += litCount;
				srcSize -= litCount;
				dest += litCount;
				destSize -= litCount;
			}
			else
			{
				const size_t repCount = static_cast<size_t>(1 - headerByte);
				if (srcSize < 1)
					return false;

				if (destSize < repCount)
					return false;

				const uint8_t repValue = *src;

				memset(dest, repValue, repCount);

				src++;
				srcSize--;
				dest += repCount;
				destSize -= repCount;
			}
		}

		return (destSize == 0);
	}

	bool QDPictDecoder::UnpackBits16(uint8_t *dest8, size_t destSize, const uint8_t *src, size_t srcSize)
	{
		uint16_t *dest16 = reinterpret_cast<uint16_t*>(dest8);
		destSize /= 2;

		while (srcSize > 0)
		{
			int8_t headerByte = *reinterpret_cast<const int8_t*>(src);
			src++;
			srcSize--;

			if (headerByte >= 0)
			{
				const size_t litCount = headerByte + 1;
				if (destSize < litCount || srcSize < litCount * 2)
					return false;

				memcpy(dest16, src, 2 * litCount);
				for (size_t i = 0; i < litCount; i++)
					ByteSwap::BigUInt16(dest16[i]);

				src += litCount * 2;
				srcSize -= litCount * 2;
				dest16 += litCount;
				destSize -= litCount;
			}
			else
			{
				const size_t repCount = static_cast<size_t>(1 - headerByte);
				if (srcSize < 2)
					return false;

				if (destSize < repCount)
					return false;

				const uint16_t repValue = (src[0] << 8) | src[1];

				for (size_t i = 0; i < repCount; i++)
					dest16[i] = repValue;

				src += 2;
				srcSize -= 2;
				dest16 += repCount;
				destSize -= repCount;
			}
		}

		return (destSize == 0);
	}
}
