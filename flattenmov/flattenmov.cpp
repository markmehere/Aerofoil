#define _CRT_SECURE_NO_WARNINGS
#include "MacBinary2.h"
#include "MacFileMem.h"
#include "CFileStream.h"
#include "MemReaderStream.h"
#include "ResourceCompiledTypeList.h"
#include "ResourceFile.h"

#include <stdio.h>

int main(int argc, const char **argv)
{
	if (argc != 3)
	{
		fprintf(stderr, "Usage: flattenmov <input.bin> <output.mov>");
		return -1;
	}

	FILE *f = fopen(argv[1], "rb");
	PortabilityLayer::CFileStream inStream(f, true, false, true);

	PortabilityLayer::MacFileMem *memFile = PortabilityLayer::MacBinary2::ReadBin(&inStream);
	inStream.Close();

	const uint8_t *dataBytes = memFile->DataFork();
	if (dataBytes[0] == 0 && dataBytes[1] == 0 && dataBytes[2] == 0 && dataBytes[3] == 0)
	{
		uint32_t mdatSize = memFile->FileInfo().m_dataForkSize;
		uint8_t mdatSizeEncoded[4];
		mdatSizeEncoded[0] = ((mdatSize >> 24) & 0xff);
		mdatSizeEncoded[1] = ((mdatSize >> 16) & 0xff);
		mdatSizeEncoded[2] = ((mdatSize >> 8) & 0xff);
		mdatSizeEncoded[3] = ((mdatSize >> 0) & 0xff);

		PortabilityLayer::ResourceFile *rf = PortabilityLayer::ResourceFile::Create();

		PortabilityLayer::MemReaderStream resStream(memFile->ResourceFork(), memFile->FileInfo().m_resourceForkSize);
		rf->Load(&resStream);

		const PortabilityLayer::ResourceCompiledTypeList *typeList = rf->GetResourceTypeList(PortabilityLayer::ResTypeID('moov'));
		const uint8_t *moovResBytes = nullptr;
		uint32_t moovResSize = 0;
		for (size_t refIndex = 0; refIndex < typeList->m_numRefs; refIndex++)
		{
			const PortabilityLayer::ResourceCompiledRef &ref = typeList->m_firstRef[refIndex];
			moovResSize = ref.GetSize();
			moovResBytes = ref.m_resData;
			break;
		}

		FILE *outF = fopen(argv[2], "wb");

		fwrite(mdatSizeEncoded, 1, 4, outF);
		fwrite(dataBytes + 4, 1, mdatSize - 4, outF);
		fwrite(moovResBytes, 1, moovResSize, outF);

		fclose(outF);
		rf->Destroy();
	}
	else
	{
		FILE *outF = fopen(argv[2], "wb");
		fwrite(dataBytes, 1, memFile->FileInfo().m_dataForkSize, outF);
		fclose(outF);
	}

	return 0;
}
