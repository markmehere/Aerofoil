#include "SoundSync.h"
#include <Windows.h>

#include <string.h>

static volatile uint64_t gs_prioritiesBlob = 0;

SoundSyncState SoundSync_ReadAll()
{
	const uint64_t priorities = gs_prioritiesBlob;

	SoundSyncState state;
	state.priority0 = static_cast<uint16_t>((priorities >> 0) & 0xffff);
	state.priority1 = static_cast<uint16_t>((priorities >> 16) & 0xffff);
	state.priority2 = static_cast<uint16_t>((priorities >> 32) & 0xffff);
	state.priority3 = static_cast<uint16_t>((priorities >> 48) & 0xffff);

	return state;
}

void SoundSync_ClearPriority(int index)
{
	const uint64_t clearMask = ~(static_cast<int64_t>(0xffff) << (index * 16));
	InterlockedAnd(&gs_prioritiesBlob, clearMask);
}

void SoundSync_PutPriority(int index, int16_t priority)
{
	const uint64_t insertMask = static_cast<int64_t>(priority) << (index * 16);
	InterlockedOr(&gs_prioritiesBlob, insertMask);
}
