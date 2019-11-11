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

OSErr GetDefaultOutputVolume(long *vol);
OSErr SetDefaultOutputVolume(long vol);


SndCallBackUPP NewSndCallBackProc(SndCallBackProc callback);
void DisposeSndCallBackUPP(SndCallBackUPP upp);
OSErr SndNewChannel(SndChannelPtr *outChannel, SndSynthType synthType, int initFlags, SndCallBackUPP callback);
OSErr SndDisposeChannel(SndChannelPtr channel, Boolean flush);
OSErr SndDoCommand(SndChannelPtr channel, const SndCommand *command, Boolean failIfFull);
OSErr SndDoImmediate(SndChannelPtr channel, const SndCommand *command);

#endif
