#include "CFileStream.h"
#include "MMHandleBlock.h"
#include "ResourceCompiledTypeList.h"
#include "ResourceFile.h"
#include "SharedTypes.h"
#include "QDStandardPalette.h"
#include "PLBigEndian.h"
#include <assert.h>

#include <string>

#include "stb_image_write.h"

// This extracts all of the color cursors from a resource file and converts them to
// Windows cursor format.
//
// Windows only supports loading color cursors by loading them from an executable or DLL resource,
// or by loading them from a file, so we can't just do the conversion/construction at runtime.
//
// Also, classic Mac cursor features are not fully supported by newer operating systems.
// The classic Mac cursor function is basically (existing ^ color) where the mask bit isn't set,
// so it supports inversion masking and colors with the same cursor.
//
// Fortunately, none of the Glider PRO cursors use this combination.
//
// The color cursor format is not fully documented.  Appears to be:
// Header
// CursorPixMapPrefix (at pixMapOffset)
// BEPixMap
// Pixel data
// BEColorTableHeader
// BEColorTableItem[...]

struct CursorPixMapPrefix
{
	BEUInt32_t m_unknown;	// Seems to always be zero
	BEUInt16_t m_subFormat;	// 0x8002 = 2 colors, 0x8004 = 4 colors, 0x8008 = 16 colors, 0x8010 = 256 colors
};

struct CursorHeader
{
	BEUInt16_t m_cursorType;
	BEUInt32_t m_pixMapOffset;
	BEUInt32_t m_pixDataOffset;
	BEUInt32_t m_expandedData;
	BEUInt16_t m_expandedDataDepth;
	BEUInt32_t m_unused;
	uint8_t m_bwCursor[32];
	uint8_t m_mask[32];
	BEUInt16_t m_hotSpotY;
	BEUInt16_t m_hotSpotX;
	BEUInt32_t m_colorTableResourceID;
	BEUInt32_t m_cursorResourceID;
};

struct IconDir
{
	uint16_t m_reserved;
	uint16_t m_type;
	uint16_t m_numImages;
};

struct IconDirEntry
{
	uint8_t m_width;
	uint8_t m_height;
	uint8_t m_numColors;
	uint8_t m_reserved;
	uint16_t m_numPlanes_HotSpotX;
	uint16_t m_bpp_HotSpotY;
	uint32_t m_imageDataSize;
	uint32_t m_imageDataOffset;
};

uint8_t CompactChannel(const uint8_t *doublet)
{
	unsigned int doubletValue = (doublet[0] << 8) + doublet[1];

	return (doubletValue * 2 + 0x101) / 0x202;
}

void WriteToFileCallback(void *context, void *data, int size)
{
	fwrite(data, 1, size, static_cast<FILE*>(context));
}

void ConvertCursors(PortabilityLayer::ResourceFile *resFile)
{
	const PortabilityLayer::ResourceCompiledTypeList *typeList = resFile->GetResourceTypeList('crsr');
	if (!typeList)
		return;

	const size_t numRefs = typeList->m_numRefs;
	for (size_t i = 0; i < numRefs; i++)
	{
		const int resID = typeList->m_firstRef[i].m_resID;
		const THandle<void> resHdl = resFile->LoadResource('crsr', resID);
		const void *cursorDataBase = *resHdl;
		const void *cursorData = cursorDataBase;

		const CursorHeader *cursorHeader = static_cast<const CursorHeader *>(cursorData);
		cursorData = cursorHeader + 1;
		cursorData = static_cast<const void*>(reinterpret_cast<const uint8_t*>(cursorDataBase) + cursorHeader->m_pixMapOffset);

		const CursorPixMapPrefix *cursorPixMapPrefix = static_cast<const CursorPixMapPrefix *>(cursorData);

		cursorData = cursorPixMapPrefix + 1;

		const BEPixMap *pixMap = reinterpret_cast<const BEPixMap*>(reinterpret_cast<const uint8_t*>(cursorData));
		cursorData = pixMap + 1;

		cursorData = static_cast<const void*>(reinterpret_cast<const uint8_t*>(cursorDataBase) + cursorHeader->m_pixDataOffset);
		const uint8_t *pixMapDataBytes = static_cast<const uint8_t*>(cursorData);

		const Rect rect = pixMap->m_bounds.ToRect();
		const int width = rect.right - rect.left;
		const int height = rect.bottom - rect.top;
		const int componentSize = static_cast<int>(pixMap->m_componentSize);

		switch (componentSize)
		{
		case 1:
		case 2:
		case 4:
		case 8:
			break;
		default:
			assert(false);
			break;
		}

		const int bitsRequired = width * height * componentSize;
		const int bytesRequired = (bitsRequired + 7) / 8;

		uint8_t *colorValues = new uint8_t[width * height];

		const int numPixels = width * height;

		switch (componentSize)
		{
		case 1:
			for (int i = 0; i < numPixels; i++)
				colorValues[i] = (pixMapDataBytes[i / 8] >> (7 - (i & 7) * 1)) & 0x1;
			break;
		case 2:
			for (int i = 0; i < numPixels; i++)
				colorValues[i] = (pixMapDataBytes[i / 4] >> (6 - (i & 3) * 2)) & 0x3;
			break;
		case 4:
			for (int i = 0; i < numPixels; i++)
				colorValues[i] = (pixMapDataBytes[i / 2] >> (4 - (i & 1) * 4)) & 0xf;
			break;
		case 8:
			for (int i = 0; i < numPixels; i++)
				colorValues[i] = pixMapDataBytes[i];
			break;
		default:
			assert(false);
			break;
		}

		cursorData = static_cast<const void*>(pixMapDataBytes + bytesRequired);
		const BEColorTableHeader *colorTableHeader = static_cast<const BEColorTableHeader *>(cursorData);

		cursorData = colorTableHeader + 1;

		const BEColorTableItem *ctabItems = static_cast<const BEColorTableItem *>(cursorData);

		uint32_t decodedCTabItems[256];
		const int numCTabItems = colorTableHeader->m_numItemsMinusOne + 1;

		for (int i = 0; i < numCTabItems; i++)
		{
			const BEColorTableItem &item = ctabItems[i];
			unsigned char rgba[4];
			rgba[0] = CompactChannel(item.m_red);
			rgba[1] = CompactChannel(item.m_green);
			rgba[2] = CompactChannel(item.m_blue);
			rgba[3] = 255;

			int index = item.m_index;
			assert(index >= 0 && index < 256);

			memcpy(decodedCTabItems + index, rgba, 4);
		}

		uint32_t *pixelDataRGBA = new uint32_t[width * height * 4];

		for (int i = 0; i < numPixels; i++)
		{
			const uint8_t ctabIndex = colorValues[i];
			pixelDataRGBA[i] = decodedCTabItems[ctabIndex];

			if (((cursorHeader->m_mask[i / 8] >> (7 - (i & 7))) & 1) == 0)
				reinterpret_cast<uint8_t*>(pixelDataRGBA + i)[3] = 0;
		}

		char outPath[64];
		sprintf_s(outPath, "Packaged\\WinCursors\\%i.cur", resID);

		FILE *outF = nullptr;
		errno_t outErr = fopen_s(&outF, outPath, "wb");

		if (!outErr)
		{
			IconDir iconDir;
			iconDir.m_reserved = 0;
			iconDir.m_type = 2;
			iconDir.m_numImages = 1;

			IconDirEntry iconDirEntry;
			iconDirEntry.m_width = width;
			iconDirEntry.m_height = height;
			iconDirEntry.m_numColors = 0;
			iconDirEntry.m_reserved = 0;
			iconDirEntry.m_numPlanes_HotSpotX = cursorHeader->m_hotSpotX;
			iconDirEntry.m_bpp_HotSpotY = cursorHeader->m_hotSpotY;
			iconDirEntry.m_imageDataSize = 0;
			iconDirEntry.m_imageDataOffset = sizeof(IconDir) + sizeof(IconDirEntry);

			fwrite(&iconDir, 1, sizeof(IconDir), outF);
			fwrite(&iconDirEntry, 1, sizeof(IconDirEntry), outF);

			long imageDataStart = ftell(outF);
			stbi_write_png_to_func(WriteToFileCallback, outF, width, height, 4, pixelDataRGBA, width * 4);
			long imageDataEnd = ftell(outF);

			fseek(outF, sizeof(IconDir), SEEK_SET);

			iconDirEntry.m_imageDataSize = static_cast<uint32_t>(imageDataEnd - imageDataStart);
			fwrite(&iconDirEntry, 1, sizeof(IconDirEntry), outF);
			fclose(outF);
		}

		delete[] colorValues;
		delete[] pixelDataRGBA;
		resHdl.Dispose();
	}
}

void ConvertIconFamily(PortabilityLayer::ResourceFile *resFile, int32_t iconBitmapType, int32_t iconColorType, const char *prefix, unsigned int dimension)
{
	const PortabilityLayer::ResourceCompiledTypeList *typeList = resFile->GetResourceTypeList(iconBitmapType);
	if (!typeList)
		return;

	const size_t numRefs = typeList->m_numRefs;
	for (size_t i = 0; i < numRefs; i++)
	{
		const int resID = typeList->m_firstRef[i].m_resID;
		const THandle<void> bwBlock = resFile->LoadResource(iconBitmapType, resID);
		const THandle<void> colorBlock = resFile->LoadResource(iconColorType, resID);

		if (!bwBlock || !colorBlock)
		{
			bwBlock.Dispose();
			colorBlock.Dispose();
			continue;
		}

		const uint8_t *bwIconData = static_cast<const uint8_t*>(*bwBlock);
		const uint8_t *bwMaskData = bwIconData + (dimension * dimension / 8);

		const uint8_t *indexedData = static_cast<const uint8_t*>(*colorBlock);

		PortabilityLayer::RGBAColor *pixelData = new PortabilityLayer::RGBAColor[dimension * dimension * 4];

		for (unsigned int i = 0; i < dimension * dimension; i++)
		{
			PortabilityLayer::RGBAColor &item = pixelData[i];

			item = PortabilityLayer::StandardPalette::GetInstance()->GetColors()[indexedData[i]];

			if (bwMaskData[i / 8] & (0x80 >> (i & 7)))
				item.a = 255;
			else
				item.a = 0;
		}

		char outPath[256];
		sprintf_s(outPath, "Aerofoil\\ConvertedResources\\%s%i.ico", prefix, resID);

		FILE *outF = nullptr;
		errno_t outErr = fopen_s(&outF, outPath, "wb");
		if (!outErr)
		{
			IconDir iconDir;
			iconDir.m_reserved = 0;
			iconDir.m_type = 1;
			iconDir.m_numImages = 1;

			IconDirEntry iconDirEntry;
			iconDirEntry.m_width = dimension;
			iconDirEntry.m_height = dimension;
			iconDirEntry.m_numColors = 0;
			iconDirEntry.m_reserved = 0;
			iconDirEntry.m_numPlanes_HotSpotX = 0;
			iconDirEntry.m_bpp_HotSpotY = 32;
			iconDirEntry.m_imageDataSize = 0;
			iconDirEntry.m_imageDataOffset = sizeof(IconDir) + sizeof(IconDirEntry);

			fwrite(&iconDir, 1, sizeof(IconDir), outF);
			fwrite(&iconDirEntry, 1, sizeof(IconDirEntry), outF);

			long imageDataStart = ftell(outF);
			stbi_write_png_to_func(WriteToFileCallback, outF, dimension, dimension, 4, pixelData, dimension * 4);
			long imageDataEnd = ftell(outF);

			fseek(outF, sizeof(IconDir), SEEK_SET);

			iconDirEntry.m_imageDataSize = static_cast<uint32_t>(imageDataEnd - imageDataStart);
			fwrite(&iconDirEntry, 1, sizeof(IconDirEntry), outF);
			fclose(outF);
		}

		delete[] pixelData;
		bwBlock.Dispose();
		colorBlock.Dispose();
	}
}

int main(int argc, const char **argv)
{
	FILE *f = nullptr;
	errno_t err = fopen_s(&f, "Packaged\\ApplicationResources.gpr", "rb");
	if (err)
		return err;

	PortabilityLayer::CFileStream stream(f);

	PortabilityLayer::ResourceFile *resFile = PortabilityLayer::ResourceFile::Create();
	if (!resFile->Load(&stream))
		return -1;

	stream.Close();

	ConvertCursors(resFile);
	ConvertIconFamily(resFile, 'ics#', 'ics8', "Small", 16);
	ConvertIconFamily(resFile, 'ICN#', 'icl8', "Large", 32);

	return 0;
}
