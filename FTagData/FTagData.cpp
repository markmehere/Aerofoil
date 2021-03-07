#include <stdio.h>
#include <string>
#include <Windows.h>
#include "MacFileInfo.h"
#include "CFileStream.h"
#include "CombinedTimestamp.h"

int main(int argc, const char **argv)
{
	if (argc < 7)
	{
		fprintf(stderr, "FTagData <timestamp> <output.gpf> <file type ID> <file creator ID> <x pos> <y pos> [flags]");
		return -1;
	}

	std::string timestampPath = argv[1];
	std::string outPath = argv[2];

	if (strlen(argv[3]) != 4)
	{
		fprintf(stderr, "File type ID must be 4 characters");
		return -2;
	}

	if (strlen(argv[4]) != 4)
	{
		fprintf(stderr, "File creator ID must be 4 characters");
		return -3;
	}

	FILE *tsF = nullptr;
	errno_t ferr = fopen_s(&tsF, timestampPath.c_str(), "rb");
	int64_t timestamp = 0;
	PortabilityLayer::CombinedTimestamp ts;

	if (!ferr)
	{
		if (fread(&ts, 1, sizeof(ts), tsF) != sizeof(ts))
		{
			fprintf(stderr, "Error reading timestamp file");
			return -1;
		}

		fclose(tsF);
	}

	PortabilityLayer::MacFileProperties mfp;
	memcpy(mfp.m_fileType, argv[3], 4);
	memcpy(mfp.m_fileCreator, argv[4], 4);
	mfp.m_xPos = atoi(argv[5]);
	mfp.m_yPos = atoi(argv[6]);
	mfp.m_finderFlags = 0;
	mfp.m_protected = 0;
	mfp.m_modifiedTimeMacEpoch = mfp.m_createdTimeMacEpoch = timestamp;

	for (int i = 7; i < argc; i++)
	{
		const char *arg = argv[i];
		if (!strcmp(arg, "locked"))
			mfp.m_finderFlags |= PortabilityLayer::FINDER_FILE_FLAG_LOCKED;
		else if (!strcmp(arg, "invisible"))
			mfp.m_finderFlags |= PortabilityLayer::FINDER_FILE_FLAG_INVISIBLE;
		else if (!strcmp(arg, "bundle"))
			mfp.m_finderFlags |= PortabilityLayer::FINDER_FILE_FLAG_BUNDLE;
		else if (!strcmp(arg, "system"))
			mfp.m_finderFlags |= PortabilityLayer::FINDER_FILE_FLAG_SYSTEM;
		else if (!strcmp(arg, "copyprotected"))
			mfp.m_finderFlags |= PortabilityLayer::FINDER_FILE_FLAG_COPY_PROTECTED;
		else if (!strcmp(arg, "busy"))
			mfp.m_finderFlags |= PortabilityLayer::FINDER_FILE_FLAG_BUSY;
		else if (!strcmp(arg, "changed"))
			mfp.m_finderFlags |= PortabilityLayer::FINDER_FILE_FLAG_CHANGED;
		else if (!strcmp(arg, "inited"))
			mfp.m_finderFlags |= PortabilityLayer::FINDER_FILE_FLAG_INITED;
	}

	PortabilityLayer::MacFilePropertiesSerialized mps;
	mps.Serialize(mfp);

	FILE *file = nullptr;
	errno_t err = fopen_s(&file, outPath.c_str(), "wb");
	if (!err)
	{
		PortabilityLayer::CFileStream stream(file);

		mps.WriteAsPackage(stream, ts);

		stream.Close();
	}

	return 0;
}
