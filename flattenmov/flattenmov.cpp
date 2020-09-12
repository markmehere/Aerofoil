#include "MacBinary2.h"
#include "MacFileMem.h"
#include "CFileStream.h"
#include "MemReaderStream.h"
#include "ResourceCompiledTypeList.h"
#include "ResourceFile.h"

#include <stdio.h>

uint8_t *ReadEntireFile(const char *path, uint32_t &szOut)
{
	FILE *f = fopen(path, "rb");
	if (!f)
	{
		fprintf(stderr, "Could not open input file '%s'", path);
		return nullptr;
	}

	PortabilityLayer::CFileStream stream(f, true, false, true);

	GpUFilePos_t sz = stream.Size();
	uint8_t *buffer = new uint8_t[sz];
	stream.Read(buffer, sz);

	stream.Close();

	szOut = static_cast<uint32_t>(sz);
	return buffer;
}

int main(int argc, const char **argv)
{
	if (argc != 4)
	{
		fprintf(stderr, "Usage: flattenmov <input.gpd> <input.gpr> <output.mov>");
		return -1;
	}

	uint32_t dataSize, resSize;
	uint8_t *dataFork = ReadEntireFile(argv[1], dataSize);
	uint8_t *resFork = ReadEntireFile(argv[2], resSize);

	if (!dataFork || !resFork)
		return -1;

	PortabilityLayer::MacFileInfo mfi;
	mfi.m_dataForkSize = dataSize;
	mfi.m_resourceForkSize = resSize;
	mfi.m_commentSize = 0;

	PortabilityLayer::MacFileMem memFile(dataFork, resFork, nullptr, mfi);

	delete[] dataFork;
	delete[] resFork;

	const uint8_t *dataBytes = memFile.DataFork();
	if (dataBytes[0] == 0 && dataBytes[1] == 0 && dataBytes[2] == 0 && dataBytes[3] == 0)
	{
		uint32_t mdatSize = memFile.FileInfo().m_dataForkSize;
		uint8_t mdatSizeEncoded[4];
		mdatSizeEncoded[0] = ((mdatSize >> 24) & 0xff);
		mdatSizeEncoded[1] = ((mdatSize >> 16) & 0xff);
		mdatSizeEncoded[2] = ((mdatSize >> 8) & 0xff);
		mdatSizeEncoded[3] = ((mdatSize >> 0) & 0xff);

		PortabilityLayer::ResourceFile *rf = PortabilityLayer::ResourceFile::Create();

		PortabilityLayer::MemReaderStream resStream(memFile.ResourceFork(), memFile.FileInfo().m_resourceForkSize);
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

		FILE *outF = fopen(argv[3], "wb");
		if (!outF)
		{
			fprintf(stderr, "Could not open output file '%s'", argv[3]);
			return -1;
		}

		fwrite(mdatSizeEncoded, 1, 4, outF);
		fwrite(dataBytes + 4, 1, mdatSize - 4, outF);
		fwrite(moovResBytes, 1, moovResSize, outF);

		fclose(outF);
		rf->Destroy();
	}
	else
	{
		FILE *outF = fopen(argv[3], "wb");
		if (!outF)
		{
			fprintf(stderr, "Could not open output file '%s'", argv[3]);
			return -1;
		}

		fwrite(dataBytes, 1, memFile.FileInfo().m_dataForkSize, outF);
		fclose(outF);
	}

	return 0;
}
