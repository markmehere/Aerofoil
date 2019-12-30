
#define _CRT_SECURE_NO_WARNINGS
//============================================================================
//----------------------------------------------------------------------------
//									Music.c
//----------------------------------------------------------------------------
//============================================================================


#include "PLResources.h"
#include "PLSound.h"
#include "Environ.h"
#include "Externs.h"
#include "SoundSync.h"
#include "HostMutex.h"
#include "HostSystemServices.h"


#define kBaseBufferMusicID			2000
#define kMaxMusic					7
#define kLastMusicPiece				16
#define kLastGamePiece				6


void MusicCallBack (SndChannelPtr, SndCommand *);
PLError_t LoadMusicSounds (void);
PLError_t DumpMusicSounds (void);
PLError_t OpenMusicChannel (void);
PLError_t CloseMusicChannel (void);


SndCallBackUPP	musicCallBackUPP;
SndChannelPtr	musicChannel;
Ptr				theMusicData[kMaxMusic];
short			musicScore[kLastMusicPiece];
short			gameScore[kLastGamePiece];
Boolean			isMusicOn, isPlayMusicIdle, isPlayMusicGame;
Boolean			failedMusic, dontLoadMusic;

// Anything accessed from this must do so under the mutex lock, unless music is stopped
struct MusicState
{
	short			musicMode;
	short			musicSoundID, musicCursor;
};

MusicState musicState;
PortabilityLayer::HostMutex *musicMutex;


extern	Boolean		isSoundOn;


//==============================================================  Functions
//--------------------------------------------------------------  StartMusic

PLError_t StartMusic (void)
{
	SndCommand	theCommand;
	PLError_t		theErr;
	short		soundVolume;
	
	theErr = PLErrors::kNone;
	
	if (dontLoadMusic)
		return(theErr);

	if (musicMutex == nullptr)
		return(theErr);
	
	UnivGetSoundVolume(&soundVolume, thisMac.hasSM3);
	
	if ((soundVolume != 0) && (!failedMusic))
	{
		theCommand.cmd = bufferCmd;
		theCommand.param1 = 0;
		theCommand.param2 = (intptr_t)(theMusicData[musicState.musicSoundID]);
		theErr = SndDoCommand(musicChannel, &theCommand, false);
		if (theErr != PLErrors::kNone)
			return (theErr);
		
		// GP: No idea what "1964" means
		theCommand.cmd = nullCmd;
		theCommand.param1 = 1964;
		theCommand.param2 = 0;
		theErr = SndDoCommand(musicChannel, &theCommand, false);
		if (theErr != PLErrors::kNone)
			return (theErr);
		
		musicState.musicCursor++;
		if (musicState.musicCursor >= kLastMusicPiece)
			musicState.musicCursor = 0;
		musicState.musicSoundID = musicScore[musicState.musicCursor];
		
		theCommand.cmd = bufferCmd;
		theCommand.param1 = 0;
		theCommand.param2 = (intptr_t)(theMusicData[musicState.musicSoundID]);
		theErr = SndDoCommand(musicChannel, &theCommand, false);
		if (theErr != PLErrors::kNone)
			return (theErr);
		
		theCommand.cmd = callBackCmd;
		theCommand.param1 = 0;
		theCommand.param2 = 0;
		theErr = SndDoCommand(musicChannel, &theCommand, false);
		
		isMusicOn = true;
	}
	
	return (theErr);
}

//--------------------------------------------------------------  StopTheMusic

void StopTheMusic (void)
{
	SndCommand	theCommand;
	PLError_t		theErr;
	
	if (dontLoadMusic)
		return;
	
	theErr = PLErrors::kNone;
	if ((isMusicOn) && (!failedMusic))
	{
		theCommand.cmd = flushCmd;
		theCommand.param1 = 0;
		theCommand.param2 = 0L;
		theErr = SndDoImmediate(musicChannel, &theCommand);
		
		theCommand.cmd = quietCmd;
		theCommand.param1 = 0;
		theCommand.param2 = 0L;
		theErr = SndDoImmediate(musicChannel, &theCommand);
		
		isMusicOn = false;
	}
}

//--------------------------------------------------------------  ToggleMusicWhilePlaying

void ToggleMusicWhilePlaying (void)
{
	PLError_t		theErr;
	
	if (dontLoadMusic)
		return;
	
	if (isPlayMusicGame)
	{
		if (!isMusicOn)
			theErr = StartMusic();
	}
	else
	{
		if (isMusicOn)
			StopTheMusic();
	}
}

//--------------------------------------------------------------  SetMusicalPiece

void SetMusicalMode (short newMode)
{	
	if (dontLoadMusic)
		return;

	musicMutex->Lock();
	switch (newMode)
	{
		case kKickGameScoreMode:
		musicState.musicCursor = 2;
		break;
		
		case kProdGameScoreMode:
		musicState.musicCursor = -1;
		break;
		
		default:
		musicState.musicMode = newMode;
		musicState.musicCursor = 0;
		break;
	}
	musicMutex->Unlock();
}

//--------------------------------------------------------------  MusicCallBack

void MusicCallBack (SndChannelPtr theChannel, SndCommand *theCommand)
{
	PLError_t		theErr;
	
//	gameA5 = theCommand.param2;
//	thisA5 = SetA5(gameA5);

	musicMutex->Lock();
	switch (musicState.musicMode)
	{
		case kPlayGameScoreMode:
		musicState.musicCursor++;
		if (musicState.musicCursor >= kLastGamePiece)
			musicState.musicCursor = 1;
		musicState.musicSoundID = gameScore[musicState.musicCursor];
		if (musicState.musicSoundID < 0)
		{
			musicState.musicCursor += musicState.musicSoundID;
			musicState.musicSoundID = gameScore[musicState.musicCursor];
		}
		break;
		
		case kPlayWholeScoreMode:
		musicState.musicCursor++;
		if (musicState.musicCursor >= kLastMusicPiece - 1)
			musicState.musicCursor = 0;
		musicState.musicSoundID = musicScore[musicState.musicCursor];
		break;
		
		default:
		musicState.musicSoundID = musicState.musicMode;
		break;
	}

	short musicSoundID = musicState.musicSoundID;
	musicMutex->Unlock();
	
	theCommand->cmd = bufferCmd;
	theCommand->param1 = 0;
	theCommand->param2 = (intptr_t)(theMusicData[musicSoundID]);
	theErr = SndDoCommand(musicChannel, theCommand, false);
	
	theCommand->cmd = callBackCmd;
	theCommand->param1 = 0;
	theCommand->param2 = 0;
	theErr = SndDoCommand(musicChannel, theCommand, false);
}

//--------------------------------------------------------------  LoadMusicSounds

PLError_t LoadMusicSounds (void)
{
	Handle		theSound;
	long		soundDataSize;
	PLError_t		theErr;
	short		i;
	
	theErr = PLErrors::kNone;
	
	for (i = 0; i < kMaxMusic; i++)
		theMusicData[i] = nil;
	
	for (i = 0; i < kMaxMusic; i++)
	{
		theSound = GetResource('snd ', i + kBaseBufferMusicID);
		if (theSound == nil)
			return PLErrors::kOutOfMemory;
		
		soundDataSize = GetHandleSize(theSound) - 20L;

		theMusicData[i] = NewPtr(soundDataSize);
		if (theMusicData[i] == nil)
			return PLErrors::kOutOfMemory;

		BlockMove((Ptr)(static_cast<Byte*>(*theSound) + 20L), theMusicData[i], soundDataSize);
		DisposeHandle(theSound);
	}
	return (theErr);
}

//--------------------------------------------------------------  DumpMusicSounds

PLError_t DumpMusicSounds (void)
{
	PLError_t		theErr;
	short		i;
	
	theErr = PLErrors::kNone;
	
	for (i = 0; i < kMaxMusic; i++)
	{
		if (theMusicData[i] != nil)
			DisposePtr(theMusicData[i]);
		theMusicData[i] = nil;
	}
	
	return (theErr);
}

//--------------------------------------------------------------  OpenMusicChannel

PLError_t OpenMusicChannel (void)
{
	PLError_t		theErr;
	
	musicCallBackUPP = NewSndCallBackProc(MusicCallBack);
	
	theErr = PLErrors::kNone;
	
	if (musicChannel != nil)
		return (theErr);
	
	musicChannel = nil;
	theErr = SndNewChannel(&musicChannel, 
			sampledSynth, initNoInterp + initMono, 
			(SndCallBackUPP)musicCallBackUPP);
	
	return (theErr);
}

//--------------------------------------------------------------  CloseMusicChannel

PLError_t CloseMusicChannel (void)
{
	PLError_t		theErr;
	
	theErr = PLErrors::kNone;
	
	if (musicChannel != nil)
		theErr = SndDisposeChannel(musicChannel, true);
	musicChannel = nil;
	
	DisposeSndCallBackUPP(musicCallBackUPP);
	
	return (theErr);
}

//--------------------------------------------------------------  InitMusic

void InitMusic (void)
{
	PLError_t		theErr;
	
	if (dontLoadMusic)
		return;
	
	musicChannel = nil;
	
	failedMusic = false;
	isMusicOn = false;
	theErr = LoadMusicSounds();
	if (theErr != PLErrors::kNone)
	{
		YellowAlert(kYellowNoMusic, theErr);
		failedMusic = true;
		return;
	}
	theErr = OpenMusicChannel();
	
	musicScore[0] = 0;
	musicScore[1] = 1;
	musicScore[2] = 2;
	musicScore[3] = 3;
	musicScore[4] = 4;
	musicScore[5] = 4;
	musicScore[6] = 0;
	musicScore[7] = 1;
	musicScore[8] = 2;
	musicScore[9] = 3;
	musicScore[10] = kPlayChorus;
	musicScore[11] = kPlayChorus;
	musicScore[12] = kPlayRefrainSparse1;
	musicScore[13] = kPlayRefrainSparse2;
	musicScore[14] = kPlayChorus;
	musicScore[15] = kPlayChorus;
	
	gameScore[0] = kPlayRefrainSparse2;
	gameScore[1] = kPlayRefrainSparse1;
	gameScore[2] = -1;
	gameScore[3] = kPlayRefrainSparse2;
	gameScore[4] = kPlayChorus;
	gameScore[5] = kPlayChorus;
	
	musicState.musicCursor = 0;
	musicState.musicSoundID = musicScore[musicState.musicCursor];
	musicState.musicMode = kPlayWholeScoreMode;

	musicMutex = PortabilityLayer::HostSystemServices::GetInstance()->CreateMutex();

	PL_NotYetImplemented_TODO("MusicSync");
	
	if (isPlayMusicIdle)
	{
		theErr = StartMusic();
		if (theErr != PLErrors::kNone)
		{
			YellowAlert(kYellowNoMusic, theErr);
			failedMusic = true;
		}
	}
}

//--------------------------------------------------------------  KillMusic

void KillMusic (void)
{
	PLError_t		theErr;
	
	if (dontLoadMusic)
		return;
	
	theErr = DumpMusicSounds();
	theErr = CloseMusicChannel();
	musicMutex->Destroy();
}

//--------------------------------------------------------------  MusicBytesNeeded

long MusicBytesNeeded (void)
{
	Handle		theSound;
	long		totalBytes;
	short		i;
	
	totalBytes = 0L;
	SetResLoad(false);
	for (i = 0; i < kMaxMusic; i++)
	{
		theSound = GetResource('snd ', i + kBaseBufferMusicID);
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

//--------------------------------------------------------------  TellHerNoMusic

void TellHerNoMusic (void)
{
	#define		kNoMemForMusicAlert	1038
	short		hitWhat;
	
//	CenterAlert(kNoMemForMusicAlert);
	hitWhat = Alert(kNoMemForMusicAlert, nil);
}

