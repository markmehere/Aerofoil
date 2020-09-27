#include "CFileStream.h"
#include "BMPFormat.h"
#include "MMHandleBlock.h"
#include "ResourceCompiledTypeList.h"
#include "ResourceFile.h"
#include "SharedTypes.h"
#include "QDStandardPalette.h"
#include "PLBigEndian.h"
#include <assert.h>

#include <string>
#include <vector>

#include "stb_image_write.h"

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

void WriteToVectorCallback(void *context, void *data, int size)
{
	std::vector<uint8_t> *vec = static_cast<std::vector<uint8_t>*>(context);
	for (int i = 0; i < size; i++)
		vec->push_back(static_cast<const uint8_t*>(data)[i]);
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

	ConvertIconFamily(resFile, 'ics#', 'ics8', "Small", 16);
	ConvertIconFamily(resFile, 'ICN#', 'icl8', "Large", 32);

	return 0;
}
