
//============================================================================
//----------------------------------------------------------------------------
//									HouseIO.c
//----------------------------------------------------------------------------
//============================================================================


#include "PLAliases.h"
#include "PLMovies.h"
#include "PLResources.h"
#include "PLStringCompare.h"
#include "PLTextUtils.h"
#include "PLPasStr.h"
#include "Externs.h"
#include "Environ.h"
#include "House.h"
#include "ObjectEdit.h"


#define kSaveChangesAlert		1002
#define kSaveChanges			1
#define kDiscardChanges			2


void LoopMovie (void);
void OpenHouseMovie (void);
void CloseHouseMovie (void);
Boolean IsFileReadOnly (FSSpec *);


Movie		theMovie;
Rect		movieRect;
short		houseRefNum, houseResFork, wasHouseVersion;
Boolean		houseOpen, fileDirty, gameDirty;
Boolean		changeLockStateOfHouse, saveHouseLocked, houseIsReadOnly;
Boolean		hasMovie, tvInRoom;

extern	FSSpecPtr	theHousesSpecs;
extern	short		thisHouseIndex, tvWithMovieNumber;
extern	short		numberRooms, housesFound;
extern	Boolean		noRoomAtAll, quitting, wardBitSet;
extern	Boolean		phoneBitSet, bannerStarCountOn;


//==============================================================  Functions
//--------------------------------------------------------------  LoopMovie

void LoopMovie (void)
{
	Handle		theLoop;
	UserData	theUserData;
	short		theCount;
	
	theLoop = NewHandle(sizeof(long));
	(** (long **) theLoop) = 0;
	theUserData = GetMovieUserData(theMovie);
	theCount = CountUserDataType(theUserData, 'LOOP');
	while (theCount--)
	{
		RemoveUserData(theUserData, 'LOOP', 1);
	}
	AddUserData(theUserData, theLoop, 'LOOP');
}

//--------------------------------------------------------------  OpenHouseMovie

void OpenHouseMovie (void)
{
#ifdef COMPILEQT
	TimeBase	theTime;
	FSSpec		theSpec;
	FInfo		finderInfo;
	Handle		spaceSaver;
	OSErr		theErr;
	short		movieRefNum;
	Boolean		dataRefWasChanged;
	
	if (thisMac.hasQT)
	{
		theSpec = theHousesSpecs[thisHouseIndex];
		PasStringConcat(theSpec.name, PSTR(".mov"));
		
		theErr = FSpGetFInfo(&theSpec, &finderInfo);
		if (theErr != noErr)
			return;
		
		theErr = OpenMovieFile(&theSpec, &movieRefNum, fsCurPerm);
		if (theErr != noErr)
		{
			YellowAlert(kYellowQTMovieNotLoaded, theErr);
			return;
		}
		
		theErr = NewMovieFromFile(&theMovie, movieRefNum, nil, theSpec.name, 
				newMovieActive, &dataRefWasChanged);
		if (theErr != noErr)
		{
			YellowAlert(kYellowQTMovieNotLoaded, theErr);
			theErr = CloseMovieFile(movieRefNum);
			return;
		}
		theErr = CloseMovieFile(movieRefNum);
		
		spaceSaver = NewHandle(307200L);
		if (spaceSaver == nil)
		{
			YellowAlert(kYellowQTMovieNotLoaded, 749);
			CloseHouseMovie();
			return;
		}
		
		GoToBeginningOfMovie(theMovie);
		theErr = LoadMovieIntoRam(theMovie, 
				GetMovieTime(theMovie, 0L), GetMovieDuration(theMovie), 0);
		if (theErr != noErr)
		{
			YellowAlert(kYellowQTMovieNotLoaded, theErr);
			DisposeHandle(spaceSaver);
			CloseHouseMovie();
			return;
		}
		DisposeHandle(spaceSaver);
				
		theErr = PrerollMovie(theMovie, 0, 0x000F0000);
		if (theErr != noErr)
		{
			YellowAlert(kYellowQTMovieNotLoaded, theErr);
			CloseHouseMovie();
			return;
		}
		
		theTime = GetMovieTimeBase(theMovie);
		SetTimeBaseFlags(theTime, loopTimeBase);
		SetMovieMasterTimeBase(theMovie, theTime, nil);
		LoopMovie();
		
		GetMovieBox(theMovie, &movieRect);
		
		hasMovie = true;
	}
#endif
}

//--------------------------------------------------------------  CloseHouseMovie

void CloseHouseMovie (void)
{
#ifdef COMPILEQT
	OSErr		theErr;
	
	if ((thisMac.hasQT) && (hasMovie))
	{
		theErr = LoadMovieIntoRam(theMovie, 
				GetMovieTime(theMovie, 0L), GetMovieDuration(theMovie), flushFromRam);
		DisposeMovie(theMovie);
	}
#endif
	hasMovie = false;
}

//--------------------------------------------------------------  OpenHouse
// Opens a house (whatever current selection is).  Returns true if all went well.

Boolean OpenHouse (void)
{
	OSErr		theErr;
	Boolean		targetIsFolder, wasAliased;
	
	if (houseOpen)
	{
		if (!CloseHouse())
			return(false);
	}
	if ((housesFound < 1) || (thisHouseIndex == -1))
		return(false);
	
	theErr = ResolveAliasFile(&theHousesSpecs[thisHouseIndex], true, 
			&targetIsFolder, &wasAliased);
	if (!CheckFileError(theErr, thisHouseName))
		return (false);
	
	#ifdef COMPILEDEMO
	if (!EqualString(theHousesSpecs[thisHouseIndex].name, "\pDemo House", false, true))
		return (false);
	#endif
	
	houseIsReadOnly = IsFileReadOnly(&theHousesSpecs[thisHouseIndex]);
	
	theErr = FSpOpenDF(&theHousesSpecs[thisHouseIndex], fsCurPerm, &houseRefNum);
	if (!CheckFileError(theErr, thisHouseName))
		return (false);
	
	houseOpen = true;
	OpenHouseResFork();
	
	hasMovie = false;
	tvInRoom = false;
	tvWithMovieNumber = -1;
	OpenHouseMovie();
	
	return (true);
}

//--------------------------------------------------------------  OpenSpecificHouse
// Opens the specific house passed in.

#ifndef COMPILEDEMO
Boolean OpenSpecificHouse (FSSpec *specs)
{
	short		i;
	Boolean		itOpened;
	
	if ((housesFound < 1) || (thisHouseIndex == -1))
		return (false);
	
	itOpened = true;
	
	for (i = 0; i < housesFound; i++)
	{
		if ((theHousesSpecs[i].vRefNum == specs->vRefNum) && 
				(theHousesSpecs[i].parID == specs->parID) && 
				(EqualString(theHousesSpecs[i].name, specs->name, false, true)))
		{
			thisHouseIndex = i;
			PasStringCopy(theHousesSpecs[thisHouseIndex].name, thisHouseName);
			if (OpenHouse())
				itOpened = ReadHouse();
			else
				itOpened = false;
			break;
		}
	}
	
	return (itOpened);
}
#endif

//--------------------------------------------------------------  SaveHouseAs

#ifndef COMPILEDEMO
Boolean SaveHouseAs (void)
{
	// TEMP - fix this later -- use NavServices (see House.c)
/*
	StandardFileReply	theReply;
	FSSpec				oldHouse;
	OSErr				theErr;
	Boolean				noProblems;
	Str255				tempStr;
	
	noProblems = true;
	
	GetLocalizedString(15, tempStr);
	StandardPutFile(tempStr, thisHouseName, &theReply);
	if (theReply.sfGood)
	{
		oldHouse = theHousesSpecs[thisHouseIndex];
			
		CloseHouseResFork();						// close this house file
		theErr = FSClose(houseRefNum);
		if (theErr != noErr)
		{
			CheckFileError(theErr, "\pPreferences");
			return(false);
		}
													// create new house file
		theErr = FSpCreate(&theReply.sfFile, 'ozm5', 'gliH', theReply.sfScript);
		if (!CheckFileError(theErr, theReply.sfFile.name))
			return (false);
		HCreateResFile(theReply.sfFile.vRefNum, theReply.sfFile.parID, 
				theReply.sfFile.name);
		if (ResError() != noErr)
			YellowAlert(kYellowFailedResCreate, ResError());
		PasStringCopy(theReply.sfFile.name, thisHouseName);
													// open new house data fork
		theErr = FSpOpenDF(&theReply.sfFile, fsRdWrPerm, &houseRefNum);
		if (!CheckFileError(theErr, thisHouseName))
			return (false);
		
		houseOpen = true;
		
		noProblems = WriteHouse(false);				// write out house data
		if (!noProblems)
			return(false);
		
		BuildHouseList();
		if (OpenSpecificHouse(&theReply.sfFile))	// open new house again
		{
		}
		else
		{
			if (OpenSpecificHouse(&oldHouse))
			{
				YellowAlert(kYellowOpenedOldHouse, 0);
			}
			else
			{
				YellowAlert(kYellowLostAllHouses, 0);
				noProblems = false;
			}
		}
	}
	
	
	return (noProblems);
	*/
	return false;
}
#endif

//--------------------------------------------------------------  ReadHouse
// With a house open, this function reads in the actual bits of data…
// into memory.

void ByteSwapPoint(Point *point)
{
	PortabilityLayer::ByteSwap::BigInt16(point->h);
	PortabilityLayer::ByteSwap::BigInt16(point->v);
}

void ByteSwapRect(Rect *rect)
{
	PortabilityLayer::ByteSwap::BigInt16(rect->top);
	PortabilityLayer::ByteSwap::BigInt16(rect->left);
	PortabilityLayer::ByteSwap::BigInt16(rect->bottom);
	PortabilityLayer::ByteSwap::BigInt16(rect->right);
}

void ByteSwapScores(scoresType *scores)
{
	for (int i = 0; i < kMaxScores; i++)
		PortabilityLayer::ByteSwap::BigInt32(scores->scores[i]);

	for (int i = 0; i < kMaxScores; i++)
		PortabilityLayer::ByteSwap::BigUInt32(scores->timeStamps[i]);

	for (int i = 0; i < kMaxScores; i++)
		PortabilityLayer::ByteSwap::BigInt16(scores->levels[i]);
}

void ByteSwapSavedGame(gameType *game)
{
	PortabilityLayer::ByteSwap::BigInt16(game->version);
	PortabilityLayer::ByteSwap::BigInt16(game->wasStarsLeft);
	PortabilityLayer::ByteSwap::BigUInt32(game->timeStamp);
	ByteSwapPoint(&game->where);
	PortabilityLayer::ByteSwap::BigInt32(game->score);
	PortabilityLayer::ByteSwap::BigInt32(game->unusedLong);
	PortabilityLayer::ByteSwap::BigInt32(game->unusedLong2);
	PortabilityLayer::ByteSwap::BigInt16(game->energy);
	PortabilityLayer::ByteSwap::BigInt16(game->bands);

	PortabilityLayer::ByteSwap::BigInt16(game->roomNumber);
	PortabilityLayer::ByteSwap::BigInt16(game->gliderState);
	PortabilityLayer::ByteSwap::BigInt16(game->numGliders);
	PortabilityLayer::ByteSwap::BigInt16(game->foil);
	PortabilityLayer::ByteSwap::BigInt16(game->unusedShort);
}

void ByteSwapBlower(blowerType *blower)
{
	ByteSwapPoint(&blower->topLeft);
	PortabilityLayer::ByteSwap::BigInt16(blower->distance);
}

void ByteSwapFurniture(furnitureType *furniture)
{
	ByteSwapRect(&furniture->bounds);
	PortabilityLayer::ByteSwap::BigInt16(furniture->pict);
}

void ByteSwapBonus(bonusType *bonus)
{
	ByteSwapPoint(&bonus->topLeft);
	PortabilityLayer::ByteSwap::BigInt16(bonus->length);
	PortabilityLayer::ByteSwap::BigInt16(bonus->points);
}

void ByteSwapTransport(transportType *transport)
{
	ByteSwapPoint(&transport->topLeft);
	PortabilityLayer::ByteSwap::BigInt16(transport->tall);
	PortabilityLayer::ByteSwap::BigInt16(transport->where);
}

void ByteSwapSwitch(switchType *sw)
{
	ByteSwapPoint(&sw->topLeft);
	PortabilityLayer::ByteSwap::BigInt16(sw->delay);
	PortabilityLayer::ByteSwap::BigInt16(sw->where);
}

void ByteSwapLight(lightType *light)
{
	ByteSwapPoint(&light->topLeft);
	PortabilityLayer::ByteSwap::BigInt16(light->length);
}

void ByteSwapAppliance(applianceType *appliance)
{
	ByteSwapPoint(&appliance->topLeft);
	PortabilityLayer::ByteSwap::BigInt16(appliance->height);
}

void ByteSwapEnemy(enemyType *enemy)
{
	ByteSwapPoint(&enemy->topLeft);
	PortabilityLayer::ByteSwap::BigInt16(enemy->length);
}

void ByteSwapClutter(clutterType *clutter)
{
	ByteSwapRect(&clutter->bounds);
	PortabilityLayer::ByteSwap::BigInt16(clutter->pict);
}

void ByteSwapObject(objectType *obj)
{
	PortabilityLayer::ByteSwap::BigInt16(obj->what);

	switch (obj->what)
	{
	case kFloorVent:
	case kCeilingVent:
	case kFloorBlower:
	case kCeilingBlower:
	case kSewerGrate:
	case kLeftFan:
	case kRightFan:
	case kTaper:
	case kCandle:
	case kStubby:
	case kTiki:
	case kBBQ:
	case kInvisBlower:
	case kGrecoVent:
	case kSewerBlower:
	case kLiftArea:
		ByteSwapBlower(&obj->data.a);
		break;

	case kTable:
	case kShelf:
	case kCabinet:
	case kFilingCabinet:
	case kWasteBasket:
	case kMilkCrate:
	case kCounter:
	case kDresser:
	case kDeckTable:
	case kStool:
	case kTrunk:
	case kInvisObstacle:
	case kManhole:
	case kBooks:
	case kInvisBounce:
		ByteSwapFurniture(&obj->data.b);
		break;

	case kRedClock:
	case kBlueClock:
	case kYellowClock:
	case kCuckoo:
	case kPaper:
	case kBattery:
	case kBands:
	case kGreaseRt:
	case kGreaseLf:
	case kFoil:
	case kInvisBonus:
	case kStar:
	case kSparkle:
	case kHelium:
	case kSlider:
		ByteSwapBonus(&obj->data.c);
		break;

	case kUpStairs:
	case kDownStairs:
	case kMailboxLf:
	case kMailboxRt:
	case kFloorTrans:
	case kCeilingTrans:
	case kDoorInLf:
	case kDoorInRt:
	case kDoorExRt:
	case kDoorExLf:
	case kWindowInLf:
	case kWindowInRt:
	case kWindowExRt:
	case kWindowExLf:
	case kInvisTrans:
	case kDeluxeTrans:
		ByteSwapTransport(&obj->data.d);
		break;

	case kLightSwitch:
	case kMachineSwitch:
	case kThermostat:
	case kPowerSwitch:
	case kKnifeSwitch:
	case kInvisSwitch:
	case kTrigger:
	case kLgTrigger:
	case kSoundTrigger:
		ByteSwapSwitch(&obj->data.e);
		break;

	case kCeilingLight:
	case kLightBulb:
	case kTableLamp:
	case kHipLamp:
	case kDecoLamp:
	case kFlourescent:
	case kTrackLight:
	case kInvisLight:
		ByteSwapLight(&obj->data.f);
		break;

	case kShredder:
	case kToaster:
	case kMacPlus:
	case kGuitar:
	case kTV:
	case kCoffee:
	case kOutlet:
	case kVCR:
	case kStereo:
	case kMicrowave:
	case kCinderBlock:
	case kFlowerBox:
	case kCDs:
	case kCustomPict:
		ByteSwapAppliance(&obj->data.g);
		break;

	case kBalloon:
	case kCopterLf:
	case kCopterRt:
	case kDartLf:
	case kDartRt:
	case kBall:
	case kDrip:
	case kFish:
	case kCobweb:
		ByteSwapEnemy(&obj->data.h);
		break;

	case kOzma:
	case kMirror:
	case kMousehole:
	case kFireplace:
	case kFlower:
	case kWallWindow:
	case kBear:
	case kCalendar:
	case kVase1:
	case kVase2:
	case kBulletin:
	case kCloud:
	case kFaucet:
	case kRug:
	case kChimes:
		ByteSwapClutter(&obj->data.i);
		break;
	default:
		break;
	};
}

void ByteSwapRoom(roomType *room)
{
	PortabilityLayer::ByteSwap::BigInt16(room->bounds);

	PortabilityLayer::ByteSwap::BigInt16(room->background);

	for (int i = 0; i < kNumTiles; i++)
		PortabilityLayer::ByteSwap::BigInt16(room->tiles[i]);

	PortabilityLayer::ByteSwap::BigInt16(room->floor);
	PortabilityLayer::ByteSwap::BigInt16(room->suite);
	PortabilityLayer::ByteSwap::BigInt16(room->openings);
	PortabilityLayer::ByteSwap::BigInt16(room->numObjects);
	for (int i = 0; i < kMaxRoomObs; i++)
		ByteSwapObject(room->objects + i);
}

bool ByteSwapHouse(housePtr house, size_t sizeInBytes)
{
	PortabilityLayer::ByteSwap::BigInt16(house->version);
	PortabilityLayer::ByteSwap::BigInt16(house->unusedShort);
	PortabilityLayer::ByteSwap::BigInt32(house->timeStamp);
	PortabilityLayer::ByteSwap::BigInt32(house->flags);
	ByteSwapPoint(&house->initial);
	ByteSwapScores(&house->highScores);
	ByteSwapSavedGame(&house->savedGame);
	PortabilityLayer::ByteSwap::BigInt16(house->firstRoom);
	PortabilityLayer::ByteSwap::BigInt16(house->nRooms);

	const size_t roomDataSize = sizeInBytes - houseType::kBinaryDataSize;
	if (house->nRooms < 1 || roomDataSize / sizeof(roomType) < static_cast<size_t>(house->nRooms))
		return false;

	const size_t nRooms = static_cast<size_t>(house->nRooms);
	for (size_t i = 0; i < nRooms; i++)
		ByteSwapRoom(house->rooms + i);

	return true;
}

Boolean ReadHouse (void)
{
	long		byteCount;
	OSErr		theErr;
	short		whichRoom;

	// There should be no padding remaining the house type
	PL_STATIC_ASSERT(sizeof(houseType) - sizeof(roomType) == houseType::kBinaryDataSize + 2);
	
	if (!houseOpen)
	{
		YellowAlert(kYellowUnaccounted, 2);
		return (false);
	}
	
	if (gameDirty || fileDirty)
	{
		if (houseIsReadOnly)
		{
			if (!WriteScoresToDisk())
			{
				YellowAlert(kYellowFailedWrite, 0);
				return(false);
			}
		}
		else if (!WriteHouse(false))
			return(false);
	}
	
	theErr = GetEOF(houseRefNum, &byteCount);
	if (theErr != noErr)
	{
		CheckFileError(theErr, thisHouseName);
		return(false);
	}
	
	#ifdef COMPILEDEMO
	if (byteCount != 16526L)
		return (false);
	#endif
	
	if (thisHouse != nil)
		DisposeHandle((Handle)thisHouse);

	// GP: Correct for padding
	const size_t alignmentPadding = sizeof(houseType) - sizeof(roomType) - houseType::kBinaryDataSize;
	
	thisHouse = (houseHand)NewHandle(byteCount + alignmentPadding);
	if (thisHouse == nil)
	{
		YellowAlert(kYellowNoMemory, 10);
		return(false);
	}
	
	theErr = SetFPos(houseRefNum, fsFromStart, 0L);
	if (theErr != noErr)
	{
		CheckFileError(theErr, thisHouseName);
		return(false);
	}
	
	long readByteCount = byteCount;
	theErr = FSRead(houseRefNum, &readByteCount, *thisHouse);
	if (theErr != noErr || readByteCount != byteCount || byteCount < static_cast<long>(houseType::kBinaryDataSize))
	{
		CheckFileError(theErr, thisHouseName);
		return(false);
	}

	if (alignmentPadding != 0)
	{
		// GP: Correct for padding
		const size_t roomDataSize = byteCount - houseType::kBinaryDataSize;

		uint8_t *houseDataBytes = reinterpret_cast<uint8_t*>(*thisHouse);
		memmove((*thisHouse)->rooms, houseDataBytes + houseType::kBinaryDataSize, roomDataSize);
	}

	ByteSwapHouse(*thisHouse, static_cast<size_t>(byteCount));
	
	numberRooms = (*thisHouse)->nRooms;
	#ifdef COMPILEDEMO
	if (numberRooms != 45)
		return (false);
	#endif
	if ((numberRooms < 1) || (byteCount == 0L))
	{
		numberRooms = 0;
		noRoomAtAll = true;
		YellowAlert(kYellowNoRooms, 0);
		return(false);
	}
	
	wasHouseVersion = (*thisHouse)->version;
	if (wasHouseVersion >= kNewHouseVersion)
	{
		YellowAlert(kYellowNewerVersion, 0);
		return(false);
	}
	
	houseUnlocked = (((*thisHouse)->timeStamp & 0x00000001) == 0);
	#ifdef COMPILEDEMO
	if (houseUnlocked)
		return (false);
	#endif
	changeLockStateOfHouse = false;
	saveHouseLocked = false;
	
	whichRoom = (*thisHouse)->firstRoom;
	#ifdef COMPILEDEMO
	if (whichRoom != 0)
		return (false);
	#endif
	
	wardBitSet = (((*thisHouse)->flags & 0x00000001) == 0x00000001);
	phoneBitSet = (((*thisHouse)->flags & 0x00000002) == 0x00000002);
	bannerStarCountOn = (((*thisHouse)->flags & 0x00000004) == 0x00000000);
	
	noRoomAtAll = (RealRoomNumberCount() == 0);
	thisRoomNumber = -1;
	previousRoom = -1;
	if (!noRoomAtAll)
		CopyRoomToThisRoom(whichRoom);
	
	if (houseIsReadOnly)
	{
		houseUnlocked = false;
		if (ReadScoresFromDisk())
		{
		}
	}
	
	objActive = kNoObjectSelected;
	ReflectCurrentRoom(true);
	gameDirty = false;
	fileDirty = false;
	UpdateMenus(false);
	
	return (true);
}

//--------------------------------------------------------------  WriteHouse
// This function writes out the house data to disk.

Boolean WriteHouse (Boolean checkIt)
{
	UInt32			timeStamp;
	long			byteCount;
	OSErr			theErr;

	PL_NotYetImplemented();
	
	if (!houseOpen)
	{
		YellowAlert(kYellowUnaccounted, 4);
		return (false);
	}
	
	theErr = SetFPos(houseRefNum, fsFromStart, 0L);
	if (theErr != noErr)
	{
		CheckFileError(theErr, thisHouseName);
		return(false);
	}
	
	CopyThisRoomToRoom();
	
	if (checkIt)
		CheckHouseForProblems();
	
	byteCount = GetHandleSize((Handle)thisHouse);
	
	if (fileDirty)
	{
		GetDateTime(&timeStamp);
		timeStamp &= 0x7FFFFFFF;
		
		if (changeLockStateOfHouse)
			houseUnlocked = !saveHouseLocked;
		
		if (houseUnlocked)								// house unlocked
			timeStamp &= 0x7FFFFFFE;
		else
			timeStamp |= 0x00000001;
		(*thisHouse)->timeStamp = (long)timeStamp;
		(*thisHouse)->version = wasHouseVersion;
	}

	ByteSwapHouse(*thisHouse, static_cast<size_t>(byteCount));

	long headerSize = houseType::kBinaryDataSize;
	long roomsSize = sizeof(roomType) * (*thisHouse)->nRooms;

	theErr = FSWrite(houseRefNum, &headerSize, *thisHouse);
	if (theErr != noErr)
	{
		CheckFileError(theErr, thisHouseName);
		ByteSwapHouse(*thisHouse, static_cast<size_t>(byteCount));
		return(false);
	}

	theErr = FSWrite(houseRefNum, &roomsSize, (*thisHouse)->rooms);
	if (theErr != noErr)
	{
		CheckFileError(theErr, thisHouseName);
		ByteSwapHouse(*thisHouse, static_cast<size_t>(byteCount));
		return(false);
	}

	ByteSwapHouse(*thisHouse, static_cast<size_t>(byteCount));

	theErr = SetEOF(houseRefNum, byteCount);
	if (theErr != noErr)
	{
		CheckFileError(theErr, thisHouseName);
		return(false);
	}
	
	if (changeLockStateOfHouse)
	{
		changeLockStateOfHouse = false;
		ReflectCurrentRoom(true);
	}
	
	gameDirty = false;
	fileDirty = false;
	UpdateMenus(false);
	return (true);
}

//--------------------------------------------------------------  CloseHouse
// This function closes the current house that is open.

Boolean CloseHouse (void)
{
	OSErr		theErr;
	
	if (!houseOpen)
		return (true);
	
	if (gameDirty)
	{
		if (houseIsReadOnly)
		{
			if (!WriteScoresToDisk())
				YellowAlert(kYellowFailedWrite, 0);
		}
		else if (!WriteHouse(theMode == kEditMode))
			YellowAlert(kYellowFailedWrite, 0);
	}
	else if (fileDirty)
	{
#ifndef COMPILEDEMO
		if (!QuerySaveChanges())	// false signifies user canceled
			return(false);
#endif
	}
	
	CloseHouseResFork();
	CloseHouseMovie();
	
	theErr = FSClose(houseRefNum);
	if (theErr != noErr)
	{
		CheckFileError(theErr, thisHouseName);
		return(false);
	}
	
	houseOpen = false;
	
	return (true);
}

//--------------------------------------------------------------  OpenHouseResFork
// Opens the resource fork of the current house that is open.

void OpenHouseResFork (void)
{
	if (houseResFork == -1)
	{
		houseResFork = FSpOpenResFile(&theHousesSpecs[thisHouseIndex], fsCurPerm);
		if (houseResFork == -1)
			YellowAlert(kYellowFailedResOpen, ResError());
		else
			UseResFile(houseResFork);
	}
}

//--------------------------------------------------------------  CloseHouseResFork
// Closes the resource fork of the current house that is open.

void CloseHouseResFork (void)
{
	if (houseResFork != -1)
	{
		CloseResFile(houseResFork);
		houseResFork = -1;
	}
}

//--------------------------------------------------------------  QuerySaveChanges
// If changes were made, this function will present the user with a…
// dialog asking them if they would like to save the changes.

#ifndef COMPILEDEMO
Boolean QuerySaveChanges (void)
{
	short		hitWhat;
	Boolean		whoCares;
	
	if (!fileDirty)
		return(true);
	
	InitCursor();
//	CenterAlert(kSaveChangesAlert);
	ParamText(thisHouseName, PSTR(""), PSTR(""), PSTR(""));
	hitWhat = Alert(kSaveChangesAlert, nil);
	if (hitWhat == kSaveChanges)
	{
		if (wasHouseVersion < kHouseVersion)
			ConvertHouseVer1To2();
		wasHouseVersion = kHouseVersion;
		if (WriteHouse(true))
			return (true);
		else
			return (false);
	}
	else if (hitWhat == kDiscardChanges)
	{
		fileDirty = false;
		if (!quitting)
		{
			whoCares = CloseHouse();
			if (OpenHouse())
				whoCares = ReadHouse();
		}
		UpdateMenus(false);
		return (true);
	}
	else
		return (false);
}
#endif

//--------------------------------------------------------------  YellowAlert
// This is a dialog used to present an error code and explanation…
// to the user when a non-lethal error has occurred.  Ideally, of…
// course, this never is called.

void YellowAlert (short whichAlert, short identifier)
{
	#define		kYellowAlert	1006
	Str255		errStr, errNumStr;
	short		whoCares;
	
	InitCursor();
	
	GetIndString(errStr, kYellowAlert, whichAlert);
	NumToString((long)identifier, errNumStr);
	
//	CenterAlert(kYellowAlert);
	ParamText(errStr, errNumStr, PSTR(""), PSTR(""));
	
	whoCares = Alert(kYellowAlert, nil);
}

//--------------------------------------------------------------  IsFileReadOnly

Boolean IsFileReadOnly (FSSpec *theSpec)
{	
	return false;
	/*
	Str255			tempStr;
	ParamBlockRec	theBlock;
	HParamBlockRec	hBlock;
	VolumeParam		*volPtr;
	OSErr			theErr;
	
	volPtr = (VolumeParam *)&theBlock;
	volPtr->ioCompletion = nil;
	volPtr->ioVolIndex = 0;
	volPtr->ioNamePtr = tempStr;
	volPtr->ioVRefNum = theSpec->vRefNum;
	
	theErr = PBGetVInfo(&theBlock, false);
	if (CheckFileError(theErr, "\pRead/Write"))
	{
		if (((volPtr->ioVAtrb & 0x0080) == 0x0080) || 
				((volPtr->ioVAtrb & 0x8000) == 0x8000))
			return (true);		// soft/hard locked bits
		else
		{
			hBlock.fileParam.ioCompletion = nil;
			hBlock.fileParam.ioVRefNum = theSpec->vRefNum;
			hBlock.fileParam.ioFVersNum = 0;
			hBlock.fileParam.ioFDirIndex = 0;
			hBlock.fileParam.ioNamePtr = theSpec->name;
			hBlock.fileParam.ioDirID = theSpec->parID;
			
			theErr = PBHGetFInfo(&hBlock, false);
			if (CheckFileError(theErr, "\pRead/Write"))
			{
				if ((hBlock.fileParam.ioFlAttrib & 0x0001) == 0x0001)
					return (true);
				else
					return (false);
			}
			else
				return (false);
		}
	}
	else
		return (false);
	*/
}

