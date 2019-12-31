
//============================================================================
//----------------------------------------------------------------------------
//									Sound.c
//----------------------------------------------------------------------------
//============================================================================


#include "PLResources.h"
#include "PLSound.h"
#include "Externs.h"
#include "MemoryManager.h"
#include "SoundSync.h"
#include "VirtualDirectory.h"


#define kBaseBufferSoundID			1000
#define kMaxSounds					64


void CallBack0 (PortabilityLayer::AudioChannel *);
void CallBack1 (PortabilityLayer::AudioChannel *);
void CallBack2 (PortabilityLayer::AudioChannel *);
PLError_t LoadBufferSounds (void);
void DumpBufferSounds (void);
PLError_t OpenSoundChannels (void);
void CloseSoundChannels (void);


PortabilityLayer::AudioChannel *channel0, *channel1, *channel2;
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
	SoundSyncState ss = SoundSync_ReadAll();
	
	if (ss.priority0 == kTriggerPriority)
	{
		channel0->ClearAllCommands();
		channel0->Stop();
	}
	
	if (ss.priority1 == kTriggerPriority)
	{
		channel1->ClearAllCommands();
		channel1->Stop();
	}
	
	if (ss.priority2 == kTriggerPriority)
	{
		channel2->ClearAllCommands();
		channel2->Stop();
	}
}

//--------------------------------------------------------------  CallBack0

void CallBack0(PortabilityLayer::AudioChannel *theChannel)
{
	SoundSync_ClearPriority(0);
}

//--------------------------------------------------------------  CallBack1

void CallBack1(PortabilityLayer::AudioChannel *theChannel)
{
	SoundSync_ClearPriority(1);
}

//--------------------------------------------------------------  CallBack2

void CallBack2(PortabilityLayer::AudioChannel *theChannel)
{
	SoundSync_ClearPriority(2);
}

//--------------------------------------------------------------  PlaySound0

void PlayExclusiveSoundChannel(short channelIndex, short soundID, short oldPriority, short newPriority)
{	
	if (failedSound || dontLoadSounds)
		return;

	PortabilityLayer::AudioChannel *channel = nil;
	PortabilityLayer::AudioChannelCallback_t callback = nil;

	switch (channelIndex)
	{
	case 0:
		channel = channel0;
		callback = CallBack0;
		break;
	case 1:
		channel = channel1;
		callback = CallBack1;
		break;
	case 2:
		channel = channel2;
		callback = CallBack2;
		break;
	default:
		return;
	}

	if (isSoundOn)
	{
		if (oldPriority != 0)
		{
			// Flush the queue and stop the channel, which will remove the pending callback
			channel->ClearAllCommands();
			channel->Stop();

			SoundSync_ClearPriority(channelIndex);
		}

		SoundSync_PutPriority(channelIndex, newPriority);

		bool succeeded = channel->AddBuffer(theSoundData[soundID], false);
		succeeded &= channel->AddCallback(callback, false);
		
		if (!succeeded)
			SoundSync_ClearPriority(channelIndex);
	}
}

//--------------------------------------------------------------  LoadTriggerSound

PLError_t LoadTriggerSound (short soundID)
{
	Handle		theSound;
	long		soundDataSize;
	PLError_t		theErr;
	
	if ((dontLoadSounds) || (theSoundData[kMaxSounds - 1] != nil))
		theErr = PLErrors::kFileNotFound;
	else
	{
//		FlushAnyTriggerPlaying();
		
		theErr = PLErrors::kNone;
		
		theSound = GetResource('snd ', soundID);
		if (theSound == nil)
		{
			theErr = PLErrors::kFileNotFound;
		}
		else
		{
			soundDataSize = GetHandleSize(theSound) - 20L;
			theSoundData[kMaxSounds - 1] = PortabilityLayer::MemoryManager::GetInstance()->Alloc(soundDataSize);
			if (theSoundData[kMaxSounds - 1] == nil)
			{
				theSound.Dispose();
				theErr = PLErrors::kOutOfMemory;
			}
			else
			{
				BlockMove((Ptr)((Byte*)(*theSound) + 20L), theSoundData[kMaxSounds - 1], soundDataSize);
				theSound.Dispose();
			}
		}
	}
	
	return (theErr);
}

//--------------------------------------------------------------  DumpTriggerSound

void DumpTriggerSound (void)
{
	if (theSoundData[kMaxSounds - 1] != nil)
		PortabilityLayer::MemoryManager::GetInstance()->Release(theSoundData[kMaxSounds - 1]);
	theSoundData[kMaxSounds - 1] = nil;
}

//--------------------------------------------------------------  LoadBufferSounds

PLError_t LoadBufferSounds (void)
{
	Handle		theSound;
	long		soundDataSize;
	PLError_t		theErr;
	short		i;
	
	theErr = PLErrors::kNone;
	
	for (i = 0; i < kMaxSounds - 1; i++)
	{
		theSound = GetResource('snd ', i + kBaseBufferSoundID);
		if (theSound == nil)
			return (PLErrors::kOutOfMemory);
		
		soundDataSize = GetHandleSize(theSound) - 20L;
		
		theSoundData[i] = PortabilityLayer::MemoryManager::GetInstance()->Alloc(soundDataSize);
		if (theSoundData[i] == nil)
			return (PLErrors::kOutOfMemory);
		
		BlockMove((Ptr)((Byte*)(*theSound) + 20L), theSoundData[i], soundDataSize);
		theSound.Dispose();
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
			PortabilityLayer::MemoryManager::GetInstance()->Release(theSoundData[i]);
		theSoundData[i] = nil;
	}
}

//--------------------------------------------------------------  OpenSoundChannels

PLError_t OpenSoundChannels (void)
{
	if (channelOpen)
		return PLErrors::kAudioError;
	
	channel0 = PortabilityLayer::SoundSystem::GetInstance()->CreateChannel();
	if (channel0)
		channelOpen = true;
	else
		return PLErrors::kAudioError;

	channel1 = PortabilityLayer::SoundSystem::GetInstance()->CreateChannel();
	if (channel1)
		channelOpen = true;
	else
		return PLErrors::kAudioError;

	channel2 = PortabilityLayer::SoundSystem::GetInstance()->CreateChannel();
	if (channel2)
		channelOpen = true;
	else
		return PLErrors::kAudioError;
	
	return PLErrors::kNone;
}

//--------------------------------------------------------------  CloseSoundChannels

void CloseSoundChannels (void)
{
	if (!channelOpen)
		return;
	
	if (channel0 != nil)
		channel0->Destroy(false);
	channel0 = nil;
	
	if (channel1 != nil)
		channel1->Destroy(false);
	channel1 = nil;
	
	if (channel2 != nil)
		channel2->Destroy(false);
	channel2 = nil;

	channelOpen = false;
}

//--------------------------------------------------------------  InitSound

void InitSound (void)
{
	PLError_t		theErr;
		
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
	if (theErr != PLErrors::kNone)
	{
		YellowAlert(kYellowFailedSound, theErr);
		failedSound = true;
	}
	
	if (!failedSound)
	{
		theErr = OpenSoundChannels();
		if (theErr != PLErrors::kNone)
		{
			YellowAlert(kYellowFailedSound, theErr);
			failedSound = true;
		}
	}
}

//--------------------------------------------------------------  KillSound

void KillSound (void)
{
	if (dontLoadSounds)
		return;
	
	DumpBufferSounds();
	CloseSoundChannels();
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
			return -1;
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

