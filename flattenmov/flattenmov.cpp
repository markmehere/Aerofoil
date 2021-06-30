#include "MacBinary2.h"
#include "MacFileMem.h"
#include "CFileStream.h"
#include "MemReaderStream.h"
#include "PLDrivers.h"
#include "PLBigEndian.h"
#include "ResourceCompiledTypeList.h"
#include "ResourceFile.h"
#include "ScopedPtr.h"
#include "GpAllocator_C.h"

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

	GpDriverCollection *drivers = PLDrivers::GetDriverCollection();
	drivers->SetDriver<GpDriverIDs::kAlloc>(GpAllocator_C::GetInstance());

	PortabilityLayer::ScopedPtr<PortabilityLayer::MacFileMem> memFile = PortabilityLayer::MacFileMem::Create(GpAllocator_C::GetInstance(), dataFork, resFork, nullptr, mfi);

	delete[] dataFork;
	delete[] resFork;

	const uint8_t *dataBytes = memFile->DataFork();

	size_t terminalAtomPos = 0;
	const size_t dataForkSize = memFile->FileInfo().m_dataForkSize;

	bool ignoreAndCopy = false;
	if (dataBytes[0] == 'F' && dataBytes[1] == 'O' && dataBytes[2] == 'R' && dataBytes[3] == 'M')
	{
		fprintf(stderr, "File appears to actually be an AIFF file\n");
		ignoreAndCopy = true;
	}

	const uint8_t *moovResBytes = nullptr;
	uint32_t moovResSize = 0;
	PortabilityLayer::ResourceFile *rf = nullptr;

	if (ignoreAndCopy)
		terminalAtomPos = dataForkSize;
	else
	{
		while (terminalAtomPos < dataForkSize)
		{
			size_t szAvailable = dataForkSize - terminalAtomPos;

			if (szAvailable < 4)
			{
				fprintf(stderr, "Error looking for terminal atom");
				return -1;
			}

			BEUInt32_t atomSize;
			memcpy(&atomSize, dataBytes + terminalAtomPos, 4);

			if (atomSize == 0)
				break;

			if (szAvailable < atomSize)
			{
				fprintf(stderr, "Error looking for terminal atom");
				return -1;
			}

			terminalAtomPos += atomSize;
		}

		rf = PortabilityLayer::ResourceFile::Create();

		if (rf)
		{

			PortabilityLayer::MemReaderStream resStream(memFile->ResourceFork(), memFile->FileInfo().m_resourceForkSize);
			rf->Load(&resStream);

			const PortabilityLayer::ResourceCompiledTypeList *typeList = rf->GetResourceTypeList(PortabilityLayer::ResTypeID('moov'));

			if (typeList != nullptr)
			{
				for (size_t refIndex = 0; refIndex < typeList->m_numRefs; refIndex++)
				{
					const PortabilityLayer::ResourceCompiledRef &ref = typeList->m_firstRef[refIndex];
					moovResSize = ref.GetSize();
					moovResBytes = ref.m_resData;
					break;
				}
			}
		}
	}

	FILE *outF = fopen(argv[3], "wb");
	if (!outF)
	{
		fprintf(stderr, "Could not open output file '%s'", argv[3]);
		return -1;
	}

	if (terminalAtomPos > 0)
		fwrite(dataBytes, 1, terminalAtomPos, outF);

	if (terminalAtomPos < dataForkSize)
	{
		BEUInt32_t atomSize(static_cast<uint32_t>(dataForkSize - terminalAtomPos));
		fwrite(&atomSize, 1, 4, outF);
		fwrite(dataBytes + terminalAtomPos + 4, 1, dataForkSize - terminalAtomPos - 4, outF);
	}

	if (moovResBytes)
		fwrite(moovResBytes, 1, moovResSize, outF);

	fclose(outF);

	if (rf)
		rf->Destroy();

	return 0;
}
