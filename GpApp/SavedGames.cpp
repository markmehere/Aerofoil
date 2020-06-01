
//============================================================================
//----------------------------------------------------------------------------
//								   SavedGames.c
//----------------------------------------------------------------------------
//============================================================================

#include "PLDialogs.h"
#include "PLStringCompare.h"
#include "DialogManager.h"
#include "Externs.h"
#include "FileManager.h"
#include "House.h"
#include "IOStream.h"
#include "InputManager.h"
#include "MacFileInfo.h"
#include "MemoryManager.h"

#include <assert.h>


#define kSavedGameVersion		0x0200


void SavedGameMismatchError (StringPtr);


gameType	smallGame;

extern	VFileSpec	*theHousesSpecs;
extern	short		numStarsRemaining, thisHouseIndex;
extern	Boolean		twoPlayerGame;


//==============================================================  Functions
//--------------------------------------------------------------  SaveGame2

void SaveGame2 (void)
{
	// Bringing up the save file UI can cause key/mouse events to be missed, resulting in state being stuck when this comes back.
	// To avoid that, clear all state here.
	PortabilityLayer::InputManager::GetInstance()->ClearState();

	Str255				gameNameStr;
	Size				byteCount;
	houseType			*thisHousePtr;
	roomType			*srcRoom;
	savedRoom			*destRoom;
	gamePtr				savedGame;
	short				r, i, numRooms;
	char				wasState;
	PortabilityLayer::IOStream	*gameStream = nullptr;

	PortabilityLayer::MemoryManager *mm = PortabilityLayer::MemoryManager::GetInstance();
	PortabilityLayer::FileManager *fm = PortabilityLayer::FileManager::GetInstance();
	
	FlushEvents();
	
	thisHousePtr = *thisHouse;
	
	numRooms = thisHousePtr->nRooms;
	
	byteCount = sizeof(game2Type) + sizeof(savedRoom) * numRooms;
	savedGame = (gamePtr)mm->Alloc(byteCount);
	if (savedGame == nil)
	{
		YellowAlert(kYellowFailedSaveGame, PLErrors::kOutOfMemory);
		return;
	}

	memset(savedGame, 0, byteCount);
	
	GetFirstWordOfString(thisHouseName, gameNameStr);
	if (gameNameStr[0] > 23)
		gameNameStr[0] = 23;
	PasStringConcat(gameNameStr, PSTR(" Game"));

	VFileSpec spec;
	spec.m_dir = PortabilityLayer::VirtualDirectories::kUserSaves;
	spec.m_name[0] = 0;

	char savePath[sizeof(spec.m_name) + 1];
	size_t savePathLength = 0;

	if (!fm->PromptSaveFile(spec.m_dir, savePath, savePathLength, sizeof(spec.m_name), PLPasStr(gameNameStr)))
	{
		mm->Release(savedGame);
		return;
	}

	assert(savePathLength < sizeof(spec.m_name) - 1);

	spec.m_name[0] = static_cast<uint8_t>(savePathLength);
	memcpy(spec.m_name + 1, savePath, savePathLength);

	if (fm->FileExists(spec.m_dir, PLPasStr(spec.m_name)))
	{
		if (!fm->DeleteFile(spec.m_dir, spec.m_name))
		{
			CheckFileError(PLErrors::kAccessDenied, PSTR("Saved Game"));
			return;
		}
	}
	
	thisHousePtr = *thisHouse;
	
	savedGame->house = theHousesSpecs[thisHouseIndex];
	savedGame->version = kSavedGameVersion;
	savedGame->wasStarsLeft = numStarsRemaining;
	savedGame->timeStamp = thisHousePtr->timeStamp;
	savedGame->where.h = theGlider.dest.left;
	savedGame->where.v = theGlider.dest.top;
	savedGame->score = theScore;
	savedGame->unusedLong = 0L;
	savedGame->unusedLong2 = 0L;
	savedGame->energy = batteryTotal;
	savedGame->bands = bandsTotal;
	savedGame->roomNumber = thisRoomNumber;
	savedGame->gliderState = theGlider.mode;
	savedGame->numGliders = mortals;
	savedGame->foil = foilTotal;
	savedGame->nRooms = numRooms;
	savedGame->facing = theGlider.facing;
	savedGame->showFoil = showFoil;
	
	for (r = 0; r < numRooms; r++)
	{
		destRoom = &(savedGame->savedData[r]);
		srcRoom = &(thisHousePtr->rooms[r]);
		
		destRoom->unusedShort = 0;
		destRoom->unusedByte = 0;
		destRoom->visited = srcRoom->visited;
		for (i = 0; i < kMaxRoomObs; i++)
			destRoom->objects[i] = srcRoom->objects[i];
	}

	PLError_t theErr = fm->CreateFileAtCurrentTime(spec.m_dir, spec.m_name, 'ozm5', 'gliG');
	if (CheckFileError(theErr, PSTR("Saved Game")))
	{
		theErr = fm->OpenFileData(spec.m_dir, spec.m_name, PortabilityLayer::EFilePermission_Write, gameStream);

		if (CheckFileError(theErr, PSTR("Saved Game")))
		{
			if (gameStream->Write(savedGame, byteCount) != byteCount)
			{
				CheckFileError(PLErrors::kIOError, PSTR("Saved Game"));
			}

			gameStream->Close();
		}
	}

	mm->Release(savedGame);
}

//--------------------------------------------------------------  SavedGameMismatchError

void SavedGameMismatchError (StringPtr gameName)
{
	#define		kSavedGameErrorAlert	1044
	short		whoCares;
	
	InitCursor();
	
//	CenterAlert(kSavedGameErrorAlert);
	DialogTextSubstitutions substitutions(gameName, thisHouseName, PSTR(""), PSTR(""));
	
	whoCares = PortabilityLayer::DialogManager::GetInstance()->DisplayAlert(kSavedGameErrorAlert, &substitutions);
}

//--------------------------------------------------------------  OpenSavedGame

Boolean OpenSavedGame (void)
{
	houseType			*thisHousePtr;
	roomType			*destRoom;
	savedRoom			*srcRoom;
	gamePtr				savedGame;
	short				r, i, gameRefNum;
	char				wasState;

	PortabilityLayer::FileManager *fm = PortabilityLayer::FileManager::GetInstance();
	PortabilityLayer::MemoryManager *mm = PortabilityLayer::MemoryManager::GetInstance();


	VFileSpec spec;
	spec.m_dir = PortabilityLayer::VirtualDirectories::kUserSaves;
	spec.m_name[0] = 0;

	char savePath[sizeof(spec.m_name) + 1];
	size_t savePathLength = 0;

	if (!fm->PromptOpenFile(spec.m_dir, savePath, savePathLength, sizeof(spec.m_name)))
		return false;

	assert(savePathLength < sizeof(spec.m_name) - 1);

	spec.m_name[0] = static_cast<uint8_t>(savePathLength);
	memcpy(spec.m_name + 1, savePath, savePathLength);

	PortabilityLayer::MacFileProperties props;
	if (!fm->ReadFileProperties(spec.m_dir, spec.m_name, props))
		return false;

	if (memcmp(props.m_fileType, "gliG", 4))
		return false;

	PortabilityLayer::IOStream *gameStream = nullptr;
	PLError_t theErr = fm->OpenFileData(spec.m_dir, spec.m_name, PortabilityLayer::EFilePermission_Read, gameStream);

	if (!CheckFileError(theErr, PSTR("Saved Game")))
		return(false);

	const PortabilityLayer::UFilePos_t fileSizeFP = gameStream->Size();
	if (fileSizeFP > SIZE_MAX)
	{
		gameStream->Close();
		return false;
	}

	const size_t byteCount = static_cast<size_t>(fileSizeFP);

	savedGame = (gamePtr)mm->Alloc(byteCount);
	if (savedGame == nil)
	{
		YellowAlert(kYellowFailedSaveGame, PLErrors::kOutOfMemory);
		gameStream->Close();
		return(false);
	}
	
	if (gameStream->Read(savedGame, byteCount) != byteCount)
	{
		CheckFileError(PLErrors::kIOError, PSTR("Saved Game"));
		mm->Release(savedGame);
		gameStream->Close();
		return(false);
	}
	
	thisHousePtr = *thisHouse;
	
	if (!StrCmp::Equal(savedGame->house.m_name, thisHouseName))
	{
		SavedGameMismatchError(savedGame->house.m_name);
		mm->Release(savedGame);
		gameStream->Close();
		return(false);
	}
	else if (thisHousePtr->timeStamp != savedGame->timeStamp)
	{
		YellowAlert(kYellowSavedTimeWrong, 0);
		mm->Release(savedGame);
		gameStream->Close();
		return(false);
	}
	else if (savedGame->version != kSavedGameVersion)
	{
		YellowAlert(kYellowSavedVersWrong, kSavedGameVersion);
		mm->Release(savedGame);
		gameStream->Close();
		return(false);
	}
	else if (savedGame->nRooms != thisHousePtr->nRooms)
	{
		YellowAlert(kYellowSavedRoomsWrong, savedGame->nRooms - thisHousePtr->nRooms);
		mm->Release(savedGame);
		gameStream->Close();
		return(false);
	}
	else
	{
		smallGame.wasStarsLeft = savedGame->wasStarsLeft;
		smallGame.where.h = savedGame->where.h;
		smallGame.where.v = savedGame->where.v;
		smallGame.score = savedGame->score;
		smallGame.unusedLong = savedGame->unusedLong;
		smallGame.unusedLong2 = savedGame->unusedLong2;
		smallGame.energy = savedGame->energy;
		smallGame.bands = savedGame->bands;
		smallGame.roomNumber = savedGame->roomNumber;
		smallGame.gliderState = savedGame->gliderState;
		smallGame.numGliders = savedGame->numGliders;
		smallGame.foil = savedGame->foil;
		smallGame.unusedShort = 0;
		smallGame.facing = savedGame->facing;
		smallGame.showFoil = savedGame->showFoil;
		
		for (r = 0; r < savedGame->nRooms; r++)
		{
			srcRoom = &(savedGame->savedData[r]);
			destRoom = &(thisHousePtr->rooms[r]);
			destRoom->visited = srcRoom->visited;
			for (i = 0; i < kMaxRoomObs; i++)
				destRoom->objects[i] = srcRoom->objects[i];
		}
	}

	mm->Release(savedGame);
	gameStream->Close();

	return (true);
}

//--------------------------------------------------------------  SaveGame

// This is probably about 3 days away from becoming the "old" functionÉ
// for saving games.
#if 0
void SaveGame (Boolean doSave)
{
	houseType		*thisHousePtr;
	UInt32			stamp;
	char			wasState;
	
	if (twoPlayerGame)
		return;
	
	thisHousePtr = *thisHouse;
	
	if (doSave)
	{
		thisHousePtr->savedGame.version = kSavedGameVersion;
		thisHousePtr->savedGame.wasStarsLeft = numStarsRemaining;
		GetDateTime(&stamp);
		thisHousePtr->savedGame.timeStamp = (long)stamp;
		thisHousePtr->savedGame.where.h = theGlider.dest.left;
		thisHousePtr->savedGame.where.v = theGlider.dest.top;
		thisHousePtr->savedGame.score = theScore;
		thisHousePtr->savedGame.unusedLong = 0L;
		thisHousePtr->savedGame.unusedLong2 = 0L;
		thisHousePtr->savedGame.energy = batteryTotal;
		thisHousePtr->savedGame.bands = bandsTotal;
		thisHousePtr->savedGame.roomNumber = thisRoomNumber;
		thisHousePtr->savedGame.gliderState = theGlider.mode;
		thisHousePtr->savedGame.numGliders = mortals;
		thisHousePtr->savedGame.foil = foilTotal;
		thisHousePtr->savedGame.unusedShort = 0;
		thisHousePtr->savedGame.facing = theGlider.facing;
		thisHousePtr->savedGame.showFoil = showFoil;
		
		thisHousePtr->hasGame = true;
	}
	else
	{
		thisHousePtr->hasGame = false;
	}
	
	if (doSave)
	{
		if (!WriteHouse(theMode == kEditMode))
			YellowAlert(kYellowFailedWrite, 0);
	}
}
#endif
