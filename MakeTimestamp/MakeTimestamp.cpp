#include <Windows.h>
#include <stdio.h>
#include <stdint.h>

int main(int argc, const char **argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "Usage: MakeTimestamp <output.ts>\n");
		fprintf(stderr, "Outputs the current timestamp in 1904 UTC epoch format");
		return -1;
	}

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
	if (!SystemTimeToFileTime(&epochStart, &epochStartFT))
		return 0;

	FILETIME timestampFT;
	GetSystemTimeAsFileTime(&timestampFT);

	int64_t epochStart64 = (static_cast<int64_t>(epochStartFT.dwLowDateTime) & 0xffffffff) | (static_cast<int64_t>(epochStartFT.dwHighDateTime) << 32);
	int64_t currentTime64 = (static_cast<int64_t>(timestampFT.dwLowDateTime) & 0xffffffff) | (static_cast<int64_t>(timestampFT.dwHighDateTime) << 32);
	int64_t timeDelta = (currentTime64 - epochStart64) / 10000000;

	FILE *f = nullptr;
	if (fopen_s(&f, argv[1], "wb"))
	{
		fprintf(stderr, "Error opening output file");
		return -1;
	}

	uint8_t encodedTimestamp[8];
	for (int i = 0; i < 8; i++)
		encodedTimestamp[i] = static_cast<uint8_t>((timeDelta >> (i * 8)) & 0xff);

	fwrite(encodedTimestamp, 8, 1, f);

	fclose(f);

	return 0;
}
