#include <stdio.h>
#include <string>
#include <Windows.h>
#include "MacFileInfo.h"

int main(int argc, const char **argv)
{
	if (argc < 7)
	{
		fprintf(stderr, "FTagData <input> <output> <file type ID> <file creator ID> <x pos> <y pos> [flags]");
		return -1;
	}

	std::string inPath = argv[1];
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

	FILETIME currentTime;
	GetSystemTimeAsFileTime(&currentTime);

	SYSTEMTIME epochStart;
	epochStart.wYear = 1904;
	epochStart.wMonth = 1;
	epochStart.wDayOfWeek = 5;
	epochStart.wDay = 1;
	epochStart.wHour = 0;
	epochStart.wMinute = 0;
	epochStart.wSecond = 0;
	epochStart.wMilliseconds = 0;

	FILETIME epochStartFT;
	SystemTimeToFileTime(&epochStart, &epochStartFT);

	int64_t epochStart64 = (static_cast<int64_t>(epochStartFT.dwLowDateTime) & 0xffffffff) | (static_cast<int64_t>(epochStartFT.dwHighDateTime) << 32);
	int64_t currentTime64 = (static_cast<int64_t>(currentTime.dwLowDateTime) & 0xffffffff) | (static_cast<int64_t>(currentTime.dwHighDateTime) << 32);

	int64_t timeDelta = (currentTime64 - epochStart64) / 10000000;

	PortabilityLayer::MacFileProperties mfp;
	memcpy(mfp.m_fileType, argv[3], 4);
	memcpy(mfp.m_fileCreator, argv[4], 4);
	mfp.m_xPos = atoi(argv[5]);
	mfp.m_yPos = atoi(argv[6]);
	mfp.m_finderFlags = 0;
	mfp.m_protected = 0;
	mfp.m_modifiedDate = mfp.m_creationDate = static_cast<uint32_t>(timeDelta & 0xffffffff);

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
