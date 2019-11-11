#include "PLSound.h"

OSErr GetDefaultOutputVolume(long *vol)
{
	short leftVol = 0x100;
	short rightVol = 0x100;

	PL_NotYetImplemented_Minor();

	*vol = (leftVol) | (rightVol << 16);

	return noErr;
}

OSErr SetDefaultOutputVolume(long vol)
{
	return noErr;
}


SndCallBackUPP NewSndCallBackProc(SndCallBackProc callback)
{
	return callback;
}

void DisposeSndCallBackUPP(SndCallBackUPP upp)
{
}

OSErr SndNewChannel(SndChannelPtr *outChannel, SndSynthType synthType, int initFlags, SndCallBackUPP callback)
{
	PL_NotYetImplemented();
	return noErr;
}

OSErr SndDisposeChannel(SndChannelPtr channel, Boolean flush)
{
	PL_NotYetImplemented();
	return noErr;
}

OSErr SndDoCommand(SndChannelPtr channel, const SndCommand *command, Boolean failIfFull)
{
	PL_NotYetImplemented();
	return noErr;
}

OSErr SndDoImmediate(SndChannelPtr channel, const SndCommand *command)
{
	PL_NotYetImplemented();
	return noErr;
}

