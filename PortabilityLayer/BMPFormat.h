#pragma once

#include "PLLittleEndian.h"

namespace PortabilityLayer
{
	namespace BitmapConstants
	{
		static const uint32_t kCompressionRGB = 0;
		static const uint32_t kCompressionBitfields = 3;
	}

	struct BitmapFileHeader
	{
		char m_id[2];	// Normally "BM"
		LEUInt32_t m_fileSize;
		LEUInt16_t m_reserved1;
		LEUInt16_t m_reserved2;
		LEUInt32_t m_imageDataStart;
	};

	struct BitmapInfoHeader
	{
		LEUInt32_t m_thisStructureSize;
		LEUInt32_t m_width;
		LEUInt32_t m_height;
		LEUInt16_t m_planes;	// Must be 1
		LEUInt16_t m_bitsPerPixel;
		LEUInt32_t m_compression;	// In V1 format, when using bitfields mode, the color table contains three DWORD color masks
		LEUInt32_t m_imageSize;
		LEUInt32_t m_xPixelsPerMeter;
		LEUInt32_t m_yPixelsPerMeter;
		LEUInt32_t m_numColors;
		LEUInt32_t m_importantColorCount;	// First N colors are important.  If 0, all are important.
	};

	struct BitmapColorTableEntry
	{
		uint8_t m_b;
		uint8_t m_g;
		uint8_t m_r;
		uint8_t m_reserved;
	};
}
