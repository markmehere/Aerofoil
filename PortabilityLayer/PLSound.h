#pragma once
#ifndef __PL_SOUND_H__
#define __PL_SOUND_H__

#include "PLCore.h"

enum SndCommandType
{
	nullCmd,
	bufferCmd,	// Param1: 0  Param2: Offset to sound header
	callBackCmd,
	flushCmd,
	quietCmd
};

struct SndChannel
{
};

struct SndCommand
{
	SndCommandType cmd;
	intptr_t param1;
	intptr_t param2;
};

enum SndSynthType
{
	sampledSynth
};

enum SndInitFlags
{
	initNoInterp = 1,
	initMono = 2,
};

typedef SndChannel *SndChannelPtr;

typedef void(*SndCallBackProc)(SndChannelPtr channel, SndCommand *command);
typedef SndCallBackProc SndCallBackUPP;

// Vol seems to be a packed stereo DWord

PLError_t GetDefaultOutputVolume(long *vol);
PLError_t SetDefaultOutputVolume(long vol);


SndCallBackUPP NewSndCallBackProc(SndCallBackProc callback);
void DisposeSndCallBackUPP(SndCallBackUPP upp);
PLError_t SndNewChannel(SndChannelPtr *outChannel, SndSynthType synthType, int initFlags, SndCallBackUPP callback);
PLError_t SndDisposeChannel(SndChannelPtr channel, Boolean flush);
PLError_t SndDoCommand(SndChannelPtr channel, const SndCommand *command, Boolean failIfFull);
PLError_t SndDoImmediate(SndChannelPtr channel, const SndCommand *command);

#endif
