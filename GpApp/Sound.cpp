
//============================================================================
//----------------------------------------------------------------------------
//									Sound.c
//----------------------------------------------------------------------------
//============================================================================


#include "PLResources.h"
#include "PLSound.h"
#include "Externs.h"
#include "SoundSync.h"


#define kBaseBufferSoundID			1000
#define kMaxSounds					64


void CallBack0 (SndChannelPtr, SndCommand *);
void CallBack1 (SndChannelPtr, SndCommand *);
void CallBack2 (SndChannelPtr, SndCommand *);
OSErr LoadBufferSounds (void);
void DumpBufferSounds (void);
OSErr OpenSoundChannels (void);
OSErr CloseSoundChannels (void);


SndCallBackUPP		callBack0UPP, callBack1UPP, callBack2UPP;
SndChannelPtr		channel0, channel1, channel2;
Ptr					theSoundData[kMaxSounds];
short				numSoundsLoaded;
Boolean				soundLoaded[kMaxSounds], dontLoadSounds;
Boolean				channelOpen, isSoundOn, failedSound;

//==============================================================  Functions
//--------------------------------------------------------------  PlayPrioritySound

void PlayPrioritySound (short which, short priority)
{
	short		lowestPriority, whosLowest;
	
	if (failedSound || dontLoadSounds)
		return;

	SoundSyncState ss = SoundSync_ReadAll();
	
	if ((priority == kTriggerPriority) && 
			((ss.priority0 == kTriggerPriority) ||
			((ss.priority1 == kTriggerPriority)) ||
			((ss.priority2 == kTriggerPriority))))
		return;
	
	whosLowest = 0;
	lowestPriority = ss.priority0;
	
	if (ss.priority1 < lowestPriority)
	{
		lowestPriority = ss.priority1;
		whosLowest = 1;
	}
	
	if (ss.priority2 < lowestPriority)
	{
		lowestPriority = ss.priority2;
		whosLowest = 2;
	}
	
	if (priority >= lowestPriority)
	{
		PlayExclusiveSoundChannel(whosLowest, which, lowestPriority, priority);
	}
}

//--------------------------------------------------------------  FlushAnyTriggerPlaying

void FlushAnyTriggerPlaying (void)
{
	SndCommand	theCommand;
	OSErr		theErr;

	SoundSyncState ss = SoundSync_ReadAll();
	
	if (ss.priority0 == kTriggerPriority)
	{
		theCommand.cmd = quietCmd;
		theCommand.param1 = 0;
		theCommand.param2 = 0;
		theErr = SndDoImmediate(channel0, &theCommand);
		theCommand.cmd = flushCmd;
		theCommand.param1 = 0;
		theCommand.param2 = 0;
		theErr = SndDoImmediate(channel0, &theCommand);
	}
	
	if (ss.priority1 == kTriggerPriority)
	{
		theCommand.cmd = quietCmd;
		theCommand.param1 = 0;
		theCommand.param2 = 0;
		theErr = SndDoImmediate(channel1, &theCommand);
		theCommand.cmd = flushCmd;
		theCommand.param1 = 0;
		theCommand.param2 = 0;
		theErr = SndDoImmediate(channel1, &theCommand);
	}
	
	if (ss.priority2 == kTriggerPriority)
	{
		theCommand.cmd = quietCmd;
		theCommand.param1 = 0;
		theCommand.param2 = 0;
		theErr = SndDoImmediate(channel2, &theCommand);
		theCommand.cmd = flushCmd;
		theCommand.param1 = 0;
		theCommand.param2 = 0;
		theErr = SndDoImmediate(channel2, &theCommand);
	}
}

//--------------------------------------------------------------  PlaySound0

void PlayExclusiveSoundChannel(short channelIndex, short soundID, short oldPriority, short newPriority)
{
	SndCommand	theCommand;
	OSErr		theErr;
	
	if (failedSound || dontLoadSounds)
		return;

	SndChannelPtr channel = nil;
	switch (channelIndex)
	{
	case 0:
		channel = channel0;
		break;
	case 1:
		channel = channel2;
		break;
	case 2:
		channel = channel2;
		break;
	default:
		return;
	}

	theErr = noErr;
	if (isSoundOn)
	{
		if (oldPriority != 0)
		{
			// Flush the queue and stop the channel, which will remove the pending callback
			theCommand.cmd = flushCmd;
			theCommand.param1 = 0;
			theCommand.param2 = 0;
			theErr = SndDoImmediate(channel, &theCommand);

			theCommand.cmd = quietCmd;
			theCommand.param1 = 0;
			theCommand.param2 = 0;
			theErr = SndDoImmediate(channel, &theCommand);

			SoundSync_ClearPriority(channelIndex);
		}

		SoundSync_PutPriority(channelIndex, newPriority);

		theCommand.cmd = bufferCmd;
		theCommand.param1 = 0;
		theCommand.param2 = (intptr_t)(theSoundData[soundID]);
		theErr = SndDoCommand(channel, &theCommand, true);
		
		theCommand.cmd = callBackCmd;
		theCommand.param1 = 0;
		theCommand.param2 = 0;
		theErr = SndDoCommand(channel, &theCommand, true);

		if (theErr != noErr)
			SoundSync_ClearPriority(channelIndex);
	}
}

//--------------------------------------------------------------  CallBack0

void CallBack0 (SndChannelPtr theChannel, SndCommand *theCommand)
{
	SoundSync_ClearPriority(0);
}

//--------------------------------------------------------------  CallBack1

void CallBack1 (SndChannelPtr theChannel, SndCommand *theCommand)
{
	SoundSync_ClearPriority(1);
}

//--------------------------------------------------------------  CallBack2

void CallBack2 (SndChannelPtr theChannel, SndCommand *theCommand)
{
	SoundSync_ClearPriority(2);
}

//--------------------------------------------------------------  LoadTriggerSound

OSErr LoadTriggerSound (short soundID)
{
	Handle		theSound;
	long		soundDataSize;
	OSErr		theErr;
	
	if ((dontLoadSounds) || (theSoundData[kMaxSounds - 1] != nil))
		theErr = -1;
	else
	{
//		FlushAnyTriggerPlaying();
		
		theErr = noErr;
		
		theSound = GetResource('snd ', soundID);
		if (theSound == nil)
		{
			theErr = -1;
		}
		else
		{
			soundDataSize = GetHandleSize(theSound) - 20L;
			theSoundData[kMaxSounds - 1] = NewPtr(soundDataSize);
			if (theSoundData[kMaxSounds - 1] == nil)
			{
				ReleaseResource(theSound);
				theErr = MemError();
			}
			else
			{
				BlockMove((Ptr)((Byte*)(*theSound) + 20L), theSoundData[kMaxSounds - 1], soundDataSize);
				ReleaseResource(theSound);
			}
		}
	}
	
	return (theErr);
}

//--------------------------------------------------------------  DumpTriggerSound

void DumpTriggerSound (void)
{
	if (theSoundData[kMaxSounds - 1] != nil)
		DisposePtr(theSoundData[kMaxSounds - 1]);
	theSoundData[kMaxSounds - 1] = nil;
}

//--------------------------------------------------------------  LoadBufferSounds

OSErr LoadBufferSounds (void)
{
	Handle		theSound;
	long		soundDataSize;
	OSErr		theErr;
	short		i;
	
	theErr = noErr;
	
	for (i = 0; i < kMaxSounds - 1; i++)
	{
		theSound = GetResource('snd ', i + kBaseBufferSoundID);
		if (theSound == nil)
			return (MemError());
		
		soundDataSize = GetHandleSize(theSound) - 20L;
		
		theSoundData[i] = NewPtr(soundDataSize);
		if (theSoundData[i] == nil)
			return (MemError());
		
		BlockMove((Ptr)((Byte*)(*theSound) + 20L), theSoundData[i], soundDataSize);
		ReleaseResource(theSound);
	}
	
	theSoundData[kMaxSounds - 1] = nil;
	
	return (theErr);
}

//--------------------------------------------------------------  DumpBufferSounds

void DumpBufferSounds (void)
{
	short		i;
	
	for (i = 0; i < kMaxSounds; i++)
	{
		if (theSoundData[i] != nil)
			DisposePtr(theSoundData[i]);
		theSoundData[i] = nil;
	}
}

//--------------------------------------------------------------  OpenSoundChannels

OSErr OpenSoundChannels (void)
{
	OSErr		theErr;
	
	callBack0UPP = NewSndCallBackProc(CallBack0);
	callBack1UPP = NewSndCallBackProc(CallBack1);
	callBack2UPP = NewSndCallBackProc(CallBack2);
	
	theErr = noErr;
	
	if (channelOpen)
		return (theErr);
	
	theErr = SndNewChannel(&channel0, 
			sampledSynth, initNoInterp + initMono, 
			(SndCallBackUPP)callBack0UPP);
	if (theErr == noErr)
		channelOpen = true;
	else
		return (theErr);
	
	theErr = SndNewChannel(&channel1, 
			sampledSynth, initNoInterp + initMono, 
			(SndCallBackUPP)callBack1UPP);
	if (theErr == noErr)
		channelOpen = true;
	else
		return (theErr);
	
	theErr = SndNewChannel(&channel2, 
			sampledSynth, initNoInterp + initMono, 
			(SndCallBackUPP)callBack2UPP);
	if (theErr == noErr)
		channelOpen = true;
	
	return (theErr);
}

//--------------------------------------------------------------  CloseSoundChannels

OSErr CloseSoundChannels (void)
{
	OSErr		theErr;
	
	theErr = noErr;
	
	if (!channelOpen)
		return (theErr);
	
	if (channel0 != nil)
		theErr = SndDisposeChannel(channel0, true);
	channel0 = nil;
	
	if (channel1 != nil)
		theErr = SndDisposeChannel(channel1, true);
	channel1 = nil;
	
	if (channel2 != nil)
		theErr = SndDisposeChannel(channel2, true);
	channel2 = nil;
	
	if (theErr == noErr)
		channelOpen = false;
	
	DisposeSndCallBackUPP(callBack0UPP);
	DisposeSndCallBackUPP(callBack1UPP);
	DisposeSndCallBackUPP(callBack2UPP);
	
	return (theErr);
}

//--------------------------------------------------------------  InitSound

void InitSound (void)
{
	OSErr		theErr;
		
	if (dontLoadSounds)
		return;
	
	failedSound = false;
	
	channel0 = nil;
	channel1 = nil;
	channel2 = nil;

	SoundSync_ClearPriority(0);
	SoundSync_ClearPriority(1);
	SoundSync_ClearPriority(2);
	
	theErr = LoadBufferSounds();
	if (theErr != noErr)
	{
		YellowAlert(kYellowFailedSound, theErr);
		failedSound = true;
	}
	
	if (!failedSound)
	{
		theErr = OpenSoundChannels();
		if (theErr != noErr)
		{
			YellowAlert(kYellowFailedSound, theErr);
			failedSound = true;
		}
	}
}

//--------------------------------------------------------------  KillSound

void KillSound (void)
{
	OSErr		theErr;
	
	if (dontLoadSounds)
		return;
	
	DumpBufferSounds();
	theErr = CloseSoundChannels();
}

//--------------------------------------------------------------  SoundBytesNeeded

long SoundBytesNeeded (void)
{
	Handle		theSound;
	long		totalBytes;
	short		i;
	
	totalBytes = 0L;
	SetResLoad(false);
	for (i = 0; i < kMaxSounds - 1; i++)
	{
		theSound = GetResource('snd ', i + kBaseBufferSoundID);
		if (theSound == nil)
		{
			SetResLoad(true);
			return ((long)ResError());
		}
		totalBytes += GetMaxResourceSize(theSound);
//		ReleaseResource(theSound);
	}
	SetResLoad(true);
	return totalBytes;
}

//--------------------------------------------------------------  TellHerNoSounds

void TellHerNoSounds (void)
{
	#define		kNoMemForSoundsAlert	1039
	short		hitWhat;
	
//	CenterAlert(kNoMemForSoundsAlert);
	hitWhat = Alert(kNoMemForSoundsAlert, nil);
}

//--------------------------------------------------------------  BitchAboutSM3

void BitchAboutSM3 (void)
{
	#define		kNoSoundManager3Alert	1030
	short		hitWhat;
	
//	CenterAlert(kNoSoundManager3Alert);
	hitWhat = Alert(kNoSoundManager3Alert, nil);
}

