#pragma once

#include <stdint.h>

struct SoundSyncState
{
	int16_t priority0;
	int16_t priority1;
	int16_t priority2;
	int16_t priority3;
};

SoundSyncState SoundSync_ReadAll();
void SoundSync_ClearPriority(int index);
void SoundSync_PutPriority(int index, int16_t priority);

