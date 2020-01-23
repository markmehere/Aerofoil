#include <stdio.h>
#include <string>
#include <Windows.h>
#include "MacFileInfo.h"

int main(int argc, const char **argv)
{
	if (argc < 8)
	{
		fprintf(stderr, "FTagData <input> <timestamp> <output> <file type ID> <file creator ID> <x pos> <y pos> [flags]");
		return -1;
	}

	std::string inPath = argv[1];
	std::string timestampPath = argv[2];
	std::string outPath = argv[3];

	if (strlen(argv[4]) != 4)
	{
		fprintf(stderr, "File type ID must be 4 characters");
		return -2;
	}

	if (strlen(argv[5]) != 4)
	{
		fprintf(stderr, "File creator ID must be 4 characters");
		return -3;
	}

	FILE *tsF = nullptr;
	errno_t ferr = fopen_s(&tsF, timestampPath.c_str(), "rb");
	int64_t timestamp = 0;

	if (!ferr)
	{
		uint8_t encodedTimestamp[8];
		if (fread(encodedTimestamp, 1, 8, tsF) != 8)
		{
			fprintf(stderr, "Error reading timestamp file");
			return -1;
		}

		for (int i = 0; i < 8; i++)
			timestamp |= static_cast<int64_t>(encodedTimestamp[i]) << (i * 8);

		fclose(tsF);
	}

	PortabilityLayer::MacFileProperties mfp;
	memcpy(mfp.m_fileType, argv[4], 4);
	memcpy(mfp.m_fileCreator, argv[5], 4);
	mfp.m_xPos = atoi(argv[6]);
	mfp.m_yPos = atoi(argv[7]);
	mfp.m_finderFlags = 0;
	mfp.m_protected = 0;
	mfp.m_modifiedDate = mfp.m_creationDate = timestamp;

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

	std::string gpdPath = outPath + ".gpd";
	CopyFile(inPath.c_str(), gpdPath.c_str(), 0);

	PortabilityLayer::MacFilePropertiesSerialized mps;
	mps.Serialize(mfp);

	std::string gpfPath = outPath + ".gpf";
	FILE *file = nullptr;
	errno_t err = fopen_s(&file, gpfPath.c_str(), "wb");
	if (!err)
	{
		fwrite(mps.m_data, PortabilityLayer::MacFilePropertiesSerialized::kSize, 1, file);
		fclose(file);
	}

	return 0;
}
