#include <Windows.h>
#include <stdio.h>
#include <stdint.h>

#include "CombinedTimestamp.h"

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

	TIME_ZONE_INFORMATION tz;
	GetTimeZoneInformation(&tz);

	SYSTEMTIME utcST;
	FileTimeToSystemTime(&timestampFT, &utcST);

	SYSTEMTIME localST;
	SystemTimeToTzSpecificLocalTime(&tz, &utcST, &localST);

	PortabilityLayer::CombinedTimestamp ts;
	ts.SetUTCTime(timeDelta);

	ts.SetLocalYear(localST.wYear);
	ts.m_localMonth = localST.wMonth;
	ts.m_localDay = localST.wDay;

	ts.m_localHour = localST.wHour;
	ts.m_localMinute = localST.wMinute;
	ts.m_localSecond = localST.wSecond;

	memset(ts.m_padding, 0, sizeof(ts.m_padding));

	fwrite(&ts, sizeof(ts), 1, f);

	fclose(f);

	return 0;
}
