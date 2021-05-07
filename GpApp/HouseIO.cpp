
//============================================================================
//----------------------------------------------------------------------------
//									HouseIO.c
//----------------------------------------------------------------------------
//============================================================================

#include "BitmapImage.h"
#include "DialogManager.h"
#include "Externs.h"
#include "Environ.h"
#include "FileManager.h"
#include "GpIOStream.h"
#include "House.h"
#include "IGpLogDriver.h"
#include "IGpSystemServices.h"
#include "ObjectEdit.h"
#include "RectUtils.h"
#include "ResourceManager.h"

#include "PLDialogs.h"
#include "PLDrivers.h"
#include "PLMovies.h"
#include "PLResources.h"
#include "PLStringCompare.h"
#include "PLPasStr.h"

#include "MacFileInfo.h"
#include "GpIOStream.h"
#include "GpVector.h"
#include "MacBinary2.h"
#include "QDPictOpcodes.h"
#include "QDPixMap.h"
#include "PLStandardColors.h"

#define kSaveChangesAlert		1002
#define kSaveChanges			1
#define kDiscardChanges			2


void OpenHouseMovie (void);
void CloseHouseMovie (void);


AnimationPlayer		theMovie;
Rect		movieRect;
PortabilityLayer::IResourceArchive	*houseResFork;
short		wasHouseVersion;
Boolean		houseOpen, fileDirty;
Boolean		changeLockStateOfHouse, saveHouseLocked, houseIsReadOnly;
Boolean		hasMovie, tvInRoom;
PortabilityLayer::CompositeFile *houseCFile;


extern	VFileSpec	*theHousesSpecs;
extern	short		thisHouseIndex, tvWithMovieNumber;
extern	short		numberRooms, housesFound;
extern	Boolean		noRoomAtAll, quitting, wardBitSet;
extern	Boolean		phoneBitSet, bannerStarCountOn;

bool ParseAndConvertSoundChecked(const THandle<void> &handle, void const*& outDataContents, size_t &outDataSize);

//==============================================================  Functions

//--------------------------------------------------------------  OpenHouseMovie

void OpenHouseMovie (void)
{
#ifdef COMPILEQT
	VFileSpec	theSpec;
	Handle		spaceSaver;
	short		movieRefNum;
	Boolean		dataRefWasChanged;
	
	if (thisMac.hasQT)
	{
		theSpec = theHousesSpecs[thisHouseIndex];
		PasStringConcat(theSpec.m_name, PSTR(".mov"));

		AnimationPackage *anim = AnimationPackage::Create();
		if (!anim)
			return;

		PLError_t theErr = anim->Load(theSpec.m_dir, theSpec.m_name);

		if (theErr != PLErrors::kNone)
		{
			anim->Destroy();

			if (theErr != PLErrors::kFileNotFound)
				YellowAlert(kYellowQTMovieNotLoaded, theErr);

			return;
		}

		movieRect = (*anim->GetFrame(0))->GetRect();
		
		hasMovie = true;
		theMovie.SetPackage(anim);

		AnimationManager::GetInstance()->RegisterPlayer(&theMovie);
	}
#endif
}

//--------------------------------------------------------------  CloseHouseMovie

void CloseHouseMovie (void)
{
#ifdef COMPILEQT
	PLError_t		theErr;
	
	if ((thisMac.hasQT) && (hasMovie))
	{
		AnimationManager::GetInstance()->RemovePlayer(&theMovie);

		theMovie.m_animPackage->Destroy();
		theMovie.m_animPackage = nullptr;
	}
#endif
	hasMovie = false;
}

//--------------------------------------------------------------  OpenHouse
// Opens a house (whatever current selection is).  Returns true if all went well.

Boolean OpenHouse (Boolean read)
{
	PLError_t		theErr;
	
	if (houseOpen)
	{
		if (!CloseHouse())
			return(false);
	}
	if ((housesFound < 1) || (thisHouseIndex == -1))
		return(false);
	
	#ifdef COMPILEDEMO
	if (!StrCmp::EqualCaseInsensitive(theHousesSpecs[thisHouseIndex].name, "\pDemo House"))
		return (false);
	#endif

	houseCFile = PortabilityLayer::FileManager::GetInstance()->OpenCompositeFile(theHousesSpecs[thisHouseIndex].m_dir, theHousesSpecs[thisHouseIndex].m_name);
	if (!houseCFile)
		return (false);

	houseIsReadOnly = houseCFile->IsDataReadOnly();

	GpIOStream *houseStream = nil;
	theErr = houseCFile->OpenData(PortabilityLayer::EFilePermission_Any, GpFileCreationDispositions::kCreateOrOpen, houseStream);
	if (!CheckFileError(theErr, thisHouseName))
	{
		houseCFile->Close();
		houseCFile = nil;
		return (false);
	}
	
	houseOpen = true;
	OpenHouseResFork();

	if (hasMovie)
		CloseHouseMovie();

	tvInRoom = false;
	tvWithMovieNumber = -1;
	OpenHouseMovie();

	if (read)
	{
		Boolean readOK = ReadHouse(houseStream, theHousesSpecs[thisHouseIndex].m_dir != PortabilityLayer::VirtualDirectories::kGameData);
		houseStream->Close();

		return readOK;
	}

	houseStream->Close();
	
	return (true);
}

//--------------------------------------------------------------  OpenSpecificHouse
// Opens the specific house passed in.

#ifndef COMPILEDEMO
Boolean OpenSpecificHouse (const VFileSpec &specs)
{
	short		i;
	Boolean		itOpened;
	
	if ((housesFound < 1) || (thisHouseIndex == -1))
		return (false);
	
	itOpened = true;
	
	for (i = 0; i < housesFound; i++)
	{
		if ((theHousesSpecs[i].m_dir == specs.m_dir) &&
				(StrCmp::EqualCaseInsensitive(theHousesSpecs[i].m_name, specs.m_name)))
		{
			thisHouseIndex = i;
			PasStringCopy(theHousesSpecs[thisHouseIndex].m_name, thisHouseName);
			if (!OpenHouse(true))
				itOpened = false;
			break;
		}
	}
	
	return (itOpened);
}
#endif


//--------------------------------------------------------------  ReadHouse
// With a house open, this function reads in the actual bits of dataÉ
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

template<size_t TSize>
void SanitizePascalStr(uint8_t(&chars)[TSize])
{
	const size_t maxLength = TSize - 1;
	size_t strLength = chars[0];
	if (strLength > maxLength)
	{
		strLength = maxLength;
		chars[0] = static_cast<uint8_t>(maxLength);
	}

	for (size_t i = 1 + strLength; i < TSize; i++)
		chars[i] = 0;
}

void ByteSwapScores(scoresType *scores)
{
	SanitizePascalStr(scores->banner);

	for (int i = 0; i < kMaxScores; i++)
		SanitizePascalStr(scores->names[i]);

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

void ByteSwapObject(objectType *obj, bool isSwappedAfter)
{
	int16_t objWhat = 0;

	if (isSwappedAfter)
		objWhat = obj->what;

	PortabilityLayer::ByteSwap::BigInt16(obj->what);

	if (!isSwappedAfter)
		objWhat = obj->what;

	switch (objWhat)
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

void ByteSwapRoom(roomType *room, bool isSwappedAfter)
{
	SanitizePascalStr(room->name);

	PortabilityLayer::ByteSwap::BigInt16(room->bounds);

	PortabilityLayer::ByteSwap::BigInt16(room->background);

	for (int i = 0; i < kNumTiles; i++)
		PortabilityLayer::ByteSwap::BigInt16(room->tiles[i]);

	PortabilityLayer::ByteSwap::BigInt16(room->floor);
	PortabilityLayer::ByteSwap::BigInt16(room->suite);
	PortabilityLayer::ByteSwap::BigInt16(room->openings);
	PortabilityLayer::ByteSwap::BigInt16(room->numObjects);
	for (int i = 0; i < kMaxRoomObs; i++)
		ByteSwapObject(room->objects + i, isSwappedAfter);
}

bool ByteSwapHouse(housePtr house, size_t sizeInBytes, bool isSwappedAfter)
{
	size_t nRooms = 0;

	if (isSwappedAfter)
		nRooms = house->nRooms;

	PortabilityLayer::ByteSwap::BigInt16(house->version);
	PortabilityLayer::ByteSwap::BigInt16(house->unusedShort);
	PortabilityLayer::ByteSwap::BigInt32(house->timeStamp);
	PortabilityLayer::ByteSwap::BigInt32(house->flags);
	ByteSwapPoint(&house->initial);
	SanitizePascalStr(house->banner);
	SanitizePascalStr(house->trailer);
	ByteSwapScores(&house->highScores);
	ByteSwapSavedGame(&house->savedGame_Unused);
	PortabilityLayer::ByteSwap::BigInt16(house->firstRoom);
	PortabilityLayer::ByteSwap::BigInt16(house->nRooms);

	if (!isSwappedAfter)
		nRooms = house->nRooms;

	const size_t roomDataSize = sizeInBytes - houseType::kBinaryDataSize;
	if (nRooms < 0 || roomDataSize / sizeof(roomType) < nRooms)
		return false;

	for (size_t i = 0; i < nRooms; i++)
		ByteSwapRoom(house->rooms + i, isSwappedAfter);

	house->padding = 0;

	return true;
}

static bool FailCheck(bool value)
{
	return value;
}

static bool SucceedCheck(bool value)
{
	return value;
}

static bool LCheck(bool value)
{
	if (!value)
		return FailCheck(value);

	return value;
}

static bool RCheck(bool value)
{
	if (value)
		return SucceedCheck(value);

	return value;
}

template<size_t TSize>
bool LegalizePascalStr(uint8_t(&chars)[TSize], bool &anyRepairs)
{
	const size_t maxLength = TSize - 1;
	if (chars[0] > maxLength)
	{
		chars[0] = static_cast<uint8_t>(maxLength);
		anyRepairs = RCheck(true);
		return true;
	}

	return true;
}

static void LegalizeBoolean(Boolean &b)
{
	if (b < 0 || b > 1)
		b = 1;
}

static bool LegalizeScores(scoresType *scores, bool &anyRepairs)
{
	if (!LegalizePascalStr(scores->banner, anyRepairs))
		return LCheck(false);

	for (int i = 0; i < kMaxScores; i++)
		if (!LegalizePascalStr(scores->names[i], anyRepairs))
			return LCheck(false);

	return true;
}

static bool LegalizeRoomLayout(houseType *house, size_t roomNum, bool &anyRepairs)
{
	roomType *room = house->rooms + roomNum;
	if (room->suite == kRoomIsEmpty)
		return true;

	if (room->suite < 0 || room->suite >= kMaxNumRoomsH)
	{
		room->suite = kRoomIsEmpty;
		anyRepairs = RCheck(true);
		return true;
	}

	if (room->floor > (kMaxNumRoomsV - kNumUndergroundFloors) || room->floor <= -kNumUndergroundFloors)
	{
		room->suite = kRoomIsEmpty;
		anyRepairs = RCheck(true);
		return true;
	}

	for (size_t ori = 0; ori < roomNum; ori++)
	{
		const roomType *otherRoom = house->rooms + ori;
		if (otherRoom->floor == room->floor && otherRoom->suite == room->suite)
		{
			room->suite = kRoomIsEmpty;
			anyRepairs = RCheck(true);
			return true;
		}
	}

	return true;
}

static void LegalizeTopLeft(Point &topLeft, bool &anyRepairs)
{
	if (topLeft.h < 0)
	{
		anyRepairs = RCheck(true);
		topLeft.h = 0;
	}
	else if (topLeft.h >= kRoomWide)
	{
		anyRepairs = RCheck(true);
		topLeft.h = kRoomWide - 1;
	}

	if (topLeft.v < 0)
	{
		anyRepairs = RCheck(true);
		topLeft.v = 0;
	}
	else if (topLeft.v >= kTileHigh)
	{
		anyRepairs = RCheck(true);
		topLeft.v = kTileHigh - 1;
	}
}

template<class TCondA, class TCondB, class TSet, class TSetTo>
void LegalizeExpect(const TCondA &condA, const TCondB &condB, TSet &set, const TSetTo &setTo, bool &anyRepairs)
{
	if (condA == condB && set != setTo)
	{
		set = setTo;
		anyRepairs = RCheck(true);
	}
}

static Boolean ForceRectInRoomRect(Rect &rect)
{
	Rect roomRect;
	QSetRect(&roomRect, 0, 0, kRoomWide, kTileHigh);

	return ForceRectInRect(&rect, &roomRect);
}


static bool LegalizeBlower(houseType *house, size_t roomNum, size_t objectNum, bool &anyRepairs)
{
	roomType *room = house->rooms + roomNum;
	objectType *obj = room->objects + objectNum;
	blowerType *blower = &obj->data.a;

	LegalizeBoolean(blower->initial);
	LegalizeBoolean(blower->state);

	int direction = -1;
	for (int i = 0; i < 4; i++)
	{
		if (blower->vector & (1 << i))
		{
			if (direction >= 0)
				anyRepairs = RCheck(true);
			else
				direction = i;
		}
	}

	static const int kDirectionUp = 0;
	static const int kDirectionRight = 1;
	static const int kDirectionDown = 2;
	static const int kDirectionLeft = 3;

	if (direction < 0 || direction > 3)
		direction = kDirectionUp;

	LegalizeTopLeft(blower->topLeft, anyRepairs);

	switch (obj->what)
	{
	case kFloorVent:
	case kFloorBlower:
	case kSewerGrate:
	case kTaper:
	case kCandle:
	case kStubby:
	case kTiki:
	case kBBQ:
	case kGrecoVent:
	case kSewerBlower:
		if (direction != kDirectionUp)
			direction = kDirectionUp;
		break;
	case kCeilingVent:
	case kCeilingBlower:
		if (direction != kDirectionDown)
			direction = kDirectionDown;
		break;
	case kRightFan:
		if (direction != kDirectionRight)
			direction = kDirectionRight;
		break;
	case kLeftFan:
		if (direction != kDirectionLeft)
			direction = kDirectionLeft;
		break;
	case kInvisBlower:
	case kLiftArea:
		break;
	default:
		return LCheck(false);
	}

	LegalizeExpect(obj->what, kFloorVent, blower->topLeft.v, kFloorVentTop, anyRepairs);
	LegalizeExpect(obj->what, kFloorBlower, blower->topLeft.v, kFloorBlowerTop, anyRepairs);
	LegalizeExpect(obj->what, kSewerGrate, blower->topLeft.v, kSewerGrateTop, anyRepairs);
	LegalizeExpect(obj->what, kCeilingVent, blower->topLeft.v, kCeilingVentTop, anyRepairs);
	LegalizeExpect(obj->what, kCeilingBlower, blower->topLeft.v, kCeilingBlowerTop, anyRepairs);

	if (blower->vector != (1 << direction))
	{
		blower->vector = static_cast<Byte>(1 << direction);

		// Unfortunately there's a lot of invalid directional data with fans, but because of how the code works, it has no effect
		//anyRepairs = RCheck(true);
	}

	if (blower->distance < 0)
	{
		blower->distance = 0;
		anyRepairs = RCheck(true);
	}

	if (obj->what == kLiftArea)
	{
		int maxWidth = kRoomWide - blower->topLeft.h;
		int maxHeight = kTileHigh - blower->topLeft.v;

		if (blower->distance > maxWidth)
		{
			blower->distance = maxWidth;
			anyRepairs = RCheck(true);
		}

		if (blower->tall * 2 > maxHeight)
		{
			blower->tall = static_cast<Byte>(maxHeight / 2);
			anyRepairs = RCheck(true);
		}

		return true;
	}

	Rect positionedRect = srcRects[obj->what];
	ZeroRectCorner(&positionedRect);
	OffsetRect(&positionedRect, blower->topLeft.h, blower->topLeft.v);

	const Rect basePositionedRect = positionedRect;

	if (ForceRectInRoomRect(positionedRect))
	{
		anyRepairs = RCheck(true);
		blower->topLeft.h = positionedRect.left;
		blower->topLeft.v = positionedRect.top;
	}

	int maxDistance = 0;
	switch (direction)
	{
	case kDirectionUp:
		{
			int highestAllowed = BlowerTypeHasUpperLimit(obj->what) ? kUpwardVentMinY : 0;
			maxDistance = blower->topLeft.v - highestAllowed;
		}
		break;
	case kDirectionRight:
		maxDistance = kRoomWide - positionedRect.right;
		break;
	case kDirectionDown:
		maxDistance = kTileHigh - positionedRect.bottom;
		break;
	case kDirectionLeft:
		maxDistance = positionedRect.left;
		break;
	default:
		assert(false);
		return LCheck(false);
	}

	if (blower->distance > maxDistance)
	{
		blower->distance = maxDistance;
		anyRepairs = RCheck(true);
	}

	return true;
}

static bool LegalizeFurniture(houseType *house, size_t roomNum, size_t objectNum, bool &anyRepairs)
{
	roomType *room = house->rooms + roomNum;
	objectType *obj = room->objects + objectNum;
	furnitureType *furniture = &obj->data.b;

	bool isVertConstrained = false;
	bool isHorizConstrained = false;
	bool isBottomConstrained = false;

	int bottomConstraint = 0;

	switch (obj->what)
	{
	default:
		assert(false);
		return LCheck(false);

	case kTable:
	case kShelf:
	case kDeckTable:
		isVertConstrained = true;
		break;
	case kBooks:
	case kFilingCabinet:
	case kWasteBasket:
	case kMilkCrate:
	case kStool:
	case kTrunk:
		isVertConstrained = true;
		isHorizConstrained = true;
		break;
	case kCabinet:
	case kInvisObstacle:
	case kInvisBounce:
		// No constraints
		break;
	case kCounter:
		isBottomConstrained = true;
		bottomConstraint = kCounterBottom;
		break;
	case kDresser:
		isBottomConstrained = true;
		bottomConstraint = kDresserBottom;
		break;
	case kManhole:
		isVertConstrained = true;
		isHorizConstrained = true;
		isBottomConstrained = true;
		bottomConstraint = kManholeSits;
		break;
	};

	const Rect baseRect = srcRects[obj->what];

	if (NormalizeRect(&furniture->bounds))
		anyRepairs = RCheck(true);

	Point topLeft = Point::Create(furniture->bounds.left, furniture->bounds.top);
	LegalizeTopLeft(topLeft, anyRepairs);

	if (obj->what == kManhole)
	{
		if (((topLeft.h - 3) % 64) != 0)
		{
			topLeft.h = (((topLeft.h + 29) / 64) * 64) + 3;
			anyRepairs = RCheck(true);
		}
	}

	uint16_t width = furniture->bounds.Width();
	uint16_t height = furniture->bounds.Height();

	if (width > kRoomWide)
	{
		width = kRoomWide;
		anyRepairs = RCheck(true);
	}

	if (height > kTileHigh)
	{
		height = kTileHigh;
		anyRepairs = RCheck(true);
	}

	LegalizeExpect(isVertConstrained, true, height, baseRect.Height(), anyRepairs);
	LegalizeExpect(isHorizConstrained, true, width, baseRect.Width(), anyRepairs);

	furniture->bounds.left = topLeft.h;
	furniture->bounds.top = topLeft.v;
	furniture->bounds.bottom = topLeft.v + static_cast<int16_t>(height);
	furniture->bounds.right = topLeft.h + static_cast<int16_t>(width);

	if (ForceRectInRoomRect(furniture->bounds))
		anyRepairs = RCheck(true);

	if (isBottomConstrained && furniture->bounds.bottom != bottomConstraint)
	{
		if (furniture->bounds.top > bottomConstraint)
			furniture->bounds.top = bottomConstraint;

		furniture->bounds.bottom = bottomConstraint;
		anyRepairs = RCheck(true);
	}

	return true;
}

static bool LegalizeBonus(houseType *house, size_t roomNum, size_t objectNum, bool &anyRepairs)
{
	roomType *room = house->rooms + roomNum;
	objectType *obj = room->objects + objectNum;
	bonusType *bonus = &obj->data.c;

	LegalizeBoolean(bonus->state);
	LegalizeBoolean(bonus->initial);

	if (obj->what == kInvisBonus && bonus->points < 0)
	{
		anyRepairs = RCheck(true);
		bonus->points = 0;
	}

	LegalizeTopLeft(bonus->topLeft, anyRepairs);

	switch (obj->what)
	{
	default:
		return LCheck(false);

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
		break;
	};

	Rect objRect = srcRects[obj->what];
	ZeroRectCorner(&objRect);

	if (obj->what == kSlider)
	{
		if (bonus->length < 0)
		{
			anyRepairs = RCheck(true);
			bonus->length = 0;
		}
		else if (bonus->length > kRoomWide)
		{
			anyRepairs = RCheck(true);
			bonus->length = kRoomWide;
		}

		objRect.right = bonus->length;
	}

	QOffsetRect(&objRect, bonus->topLeft.h, bonus->topLeft.v);

	if (ForceRectInRoomRect(objRect))
	{
		anyRepairs = RCheck(true);
		bonus->topLeft.h = objRect.left;
		bonus->topLeft.v = objRect.top;
	}

	int maxLength = -1;
	if (obj->what == kGreaseRt)
		maxLength = kRoomWide - objRect.right;

	if (obj->what == kGreaseLf)
		maxLength = objRect.left;

	if (obj->what == kSlider)
		maxLength = kRoomWide - bonus->topLeft.h;

	if (maxLength >= 0)
	{
		if (bonus->length < 0)
		{
			anyRepairs = RCheck(true);
			bonus->length = 0;
		}
		else if (bonus->length > maxLength)
		{
			anyRepairs = RCheck(true);
			bonus->length = maxLength;
		}
	}

	return true;
}

static void LegalizeRoomLink(houseType *house, int16_t &where, bool &anyRepairs)
{
	// Leftover -1 from version conversion, these are generally valid
	if (where == -100)
		where = -1;

	if (where < -1)
	{
		anyRepairs = RCheck(true);
		where = -1;
	}
	else if (where >= 0)
	{
		SInt16 floor, suite;
		ExtractFloorSuite(house, where, &floor, &suite);

		if (suite < 0 || suite >= kMaxNumRoomsH)
		{
			anyRepairs = RCheck(true);
			where = -1;
		}

		if (floor > (kMaxNumRoomsV - kNumUndergroundFloors) || floor <= -kNumUndergroundFloors)
		{
			anyRepairs = RCheck(true);
			where = -1;
		}
	}
}

static bool LegalizeTransport(houseType *house, size_t roomNum, size_t objectNum, bool &anyRepairs)
{
	roomType *room = house->rooms + roomNum;
	objectType *obj = room->objects + objectNum;
	transportType *transport = &obj->data.d;

	LegalizeTopLeft(transport->topLeft, anyRepairs);
	LegalizeRoomLink(house, transport->where, anyRepairs);

	Rect objRect = srcRects[obj->what];
	ZeroRectCorner(&objRect);

	if (obj->what == kInvisTrans)
	{
		if (transport->tall < 0)
		{
			anyRepairs = RCheck(true);
			transport->tall = 0;
		}
		else if (transport->tall > kTileHigh)
		{
			anyRepairs = RCheck(true);
			transport->tall = kTileHigh;
		}

		objRect.right = transport->wide;
		objRect.bottom = transport->tall;
	}


	if (obj->what == kDeluxeTrans)
	{
		uint8_t codedWidth = static_cast<uint8_t>((transport->tall >> 8) & 0xff);
		uint8_t codedHeight = static_cast<uint8_t>((transport->tall) & 0xff);

		objRect.right = codedWidth * 4;
		objRect.bottom = codedHeight * 4;
	}

	QOffsetRect(&objRect, transport->topLeft.h, transport->topLeft.v);

	if (ForceRectInRoomRect(objRect))
	{
		anyRepairs = RCheck(true);
		transport->topLeft.h = objRect.left;
		transport->topLeft.v = objRect.top;

		if (obj->what == kDeluxeTrans)
			transport->tall = static_cast<int16_t>(((objRect.Width() / 4) << 8) + objRect.Height() / 4);
		else if (obj->what == kInvisTrans)
		{
			transport->wide = objRect.Width();
			transport->tall = objRect.Height();
		}
	}

	switch (obj->what)
	{
	default:
		return LCheck(false);

	case kUpStairs:
	case kDownStairs:
		if (transport->topLeft.v != kStairsTop)
		{
			anyRepairs = RCheck(true);
			transport->topLeft.v = kStairsTop;
		}
		break;
	case kFloorTrans:
		if (transport->topLeft.v != kFloorTransTop)
		{
			anyRepairs = RCheck(true);
			transport->topLeft.v = kFloorTransTop;
		}
		break;
	case kCeilingTrans:
		if (transport->topLeft.v != kCeilingTransTop)
		{
			anyRepairs = RCheck(true);
			transport->topLeft.v = kCeilingTransTop;
		}
		break;
	case kDoorInLf:
		if (transport->topLeft.h != kDoorInLfLeft)
		{
			anyRepairs = RCheck(true);
			transport->topLeft.h = kCeilingTransTop;
		}
		if (transport->topLeft.v != kDoorInTop)
		{
			anyRepairs = RCheck(true);
			transport->topLeft.v = kDoorInTop;
		}
		break;
	case kDoorInRt:
		if (transport->topLeft.h != kDoorInRtLeft)
		{
			anyRepairs = RCheck(true);
			transport->topLeft.h = kDoorInRtLeft;
		}
		if (transport->topLeft.v != kDoorInTop)
		{
			anyRepairs = RCheck(true);
			transport->topLeft.v = kDoorInTop;
		}
		break;
	case kDoorExRt:
		if (transport->topLeft.h != kDoorExRtLeft)
		{
			anyRepairs = RCheck(true);
			transport->topLeft.h = kDoorExRtLeft;
		}
		if (transport->topLeft.v != kDoorExTop)
		{
			anyRepairs = RCheck(true);
			transport->topLeft.v = kDoorExTop;
		}
		break;
	case kDoorExLf:
		if (transport->topLeft.h != kDoorExLfLeft)
		{
			anyRepairs = RCheck(true);
			transport->topLeft.h = kDoorExLfLeft;
		}
		if (transport->topLeft.v != kDoorExTop)
		{
			anyRepairs = RCheck(true);
			transport->topLeft.v = kDoorExTop;
		}
		break;
	case kWindowInLf:
		if (transport->topLeft.h != kWindowInLfLeft)
		{
			anyRepairs = RCheck(true);
			transport->topLeft.h = kWindowInLfLeft;
		}
		if (transport->topLeft.v != kWindowInTop)
		{
			anyRepairs = RCheck(true);
			transport->topLeft.v = kWindowInTop;
		}
		break;
	case kWindowInRt:
		if (transport->topLeft.h != kWindowInRtLeft)
		{
			anyRepairs = RCheck(true);
			transport->topLeft.h = kWindowInRtLeft;
		}
		if (transport->topLeft.v != kWindowInTop)
		{
			anyRepairs = RCheck(true);
			transport->topLeft.v = kWindowInTop;
		}
		break;
	case kWindowExRt:
		if (transport->topLeft.h != kWindowExRtLeft)
		{
			anyRepairs = RCheck(true);
			transport->topLeft.h = kWindowExRtLeft;
		}
		if (transport->topLeft.v != kWindowExTop)
		{
			anyRepairs = RCheck(true);
			transport->topLeft.v = kWindowExTop;
		}
		break;
	case kWindowExLf:
		if (transport->topLeft.h != kWindowExLfLeft)
		{
			anyRepairs = RCheck(true);
			transport->topLeft.h = kWindowExLfLeft;
		}
		if (transport->topLeft.v != kWindowExTop)
		{
			anyRepairs = RCheck(true);
			transport->topLeft.v = kWindowExTop;
		}
		break;
	case kInvisTrans:
	case kDeluxeTrans:
	case kMailboxLf:
	case kMailboxRt:
		break;
	}

	return true;
}

static bool LegalizeSwitch(houseType *house, size_t roomNum, size_t objectNum, bool &anyRepairs)
{
	roomType *room = house->rooms + roomNum;
	objectType *obj = room->objects + objectNum;
	switchType *sw = &obj->data.e;

	if (sw->delay < 0)
	{
		sw->delay = 0;
		anyRepairs = RCheck(true);
	}

	bool isSwitch = false;
	bool isTrigger = false;

	switch (obj->what)
	{
	default:
		return RCheck(false);

	case kLightSwitch:
	case kMachineSwitch:
	case kThermostat:
	case kPowerSwitch:
	case kKnifeSwitch:
	case kInvisSwitch:
		isSwitch = true;
		LegalizeRoomLink(house, sw->where, anyRepairs);
		break;
	case kTrigger:
	case kLgTrigger:
		LegalizeRoomLink(house, sw->where, anyRepairs);
		break;
	case kSoundTrigger:
		break;
	};

	if (isSwitch)
	{
		switch (sw->type)
		{
		case kToggle:
		case kForceOn:
		case kForceOff:
			break;
		default:
			anyRepairs = RCheck(true);
			sw->type = kToggle;
			break;
		}
	}

	if (isTrigger)
	{
		if (sw->type != kOneShot)
		{
			anyRepairs = RCheck(true);
			sw->type = kOneShot;
		}
	}

	LegalizeTopLeft(sw->topLeft, anyRepairs);

	Rect bounds = srcRects[obj->what];
	ZeroRectCorner(&bounds);
	QOffsetRect(&bounds, sw->topLeft.h, sw->topLeft.v);

	if (ForceRectInRoomRect(bounds))
	{
		anyRepairs = RCheck(true);
		sw->topLeft.h = bounds.left;
		sw->topLeft.v = bounds.top;
	}

	return true;
}

static bool LegalizeLight(houseType *house, size_t roomNum, size_t objectNum, bool &anyRepairs)
{
	roomType *room = house->rooms + roomNum;
	objectType *obj = room->objects + objectNum;
	lightType *light = &obj->data.f;

	LegalizeBoolean(light->initial);
	LegalizeBoolean(light->state);

	LegalizeTopLeft(light->topLeft, anyRepairs);

	Rect bounds = srcRects[obj->what];
	ZeroRectCorner(&bounds);

	if (obj->what == kTrackLight || obj->what == kFlourescent)
	{
		if (light->length < 0)
		{
			anyRepairs = RCheck(true);
			light->length = 0;
		}
		else if (light->length > kRoomWide)
		{
			anyRepairs = RCheck(true);
			light->length = kRoomWide;
		}

		bounds.right = light->length;
	}

	QOffsetRect(&bounds, light->topLeft.h, light->topLeft.v);

	if (ForceRectInRoomRect(bounds))
	{
		anyRepairs = RCheck(true);
		light->topLeft.h = bounds.left;
		light->topLeft.v = bounds.top;
	}

	switch (obj->what)
	{
	case kCeilingLight:
		if (light->topLeft.v != kCeilingLightTop)
		{
			anyRepairs = RCheck(true);
			light->topLeft.v = kCeilingLightTop;
		}
		break;
	case kHipLamp:
		if (light->topLeft.v != kHipLampTop)
		{
			anyRepairs = RCheck(true);
			light->topLeft.v = kHipLampTop;
		}
		break;
	case kDecoLamp:
		if (light->topLeft.v != kDecoLampTop)
		{
			anyRepairs = RCheck(true);
			light->topLeft.v = kDecoLampTop;
		}
		break;
	case kFlourescent:
		if (light->topLeft.v != kFlourescentTop)
		{
			anyRepairs = RCheck(true);
			light->topLeft.v = kFlourescentTop;
		}
		break;
	case kTrackLight:
		if (light->topLeft.v != kTrackLightTop)
		{
			anyRepairs = RCheck(true);
			light->topLeft.v = kTrackLightTop;
		}
		break;
	case kLightBulb:
	case kTableLamp:
	case kInvisLight:
		break;
	}

	return true;
}

static bool LegalizeAppliance(houseType *house, size_t roomNum, size_t objectNum, bool &anyRepairs)
{
	roomType *room = house->rooms + roomNum;
	objectType *obj = room->objects + objectNum;
	applianceType *appl = &obj->data.g;

	LegalizeBoolean(appl->initial);
	LegalizeBoolean(appl->state);

	LegalizeTopLeft(appl->topLeft, anyRepairs);

	if (obj->what != kCustomPict)
	{
		Rect bounds = srcRects[obj->what];
		ZeroRectCorner(&bounds);
		QOffsetRect(&bounds, appl->topLeft.h, appl->topLeft.v);

		if (ForceRectInRoomRect(bounds))
		{
			anyRepairs = RCheck(true);
			appl->topLeft.h = bounds.left;
			appl->topLeft.v = bounds.top;
		}

		switch (obj->what)
		{
		case kToaster:
			{
				int maxHeight = bounds.top;
				if (appl->height < 0)
				{
					anyRepairs = RCheck(true);
					appl->height = 0;
				}
				else if (appl->height > maxHeight)
				{
					anyRepairs = RCheck(true);
					appl->height = maxHeight;
				}
			}
			break;
		case kShredder:
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
			break;
		}
	}

	return true;
}

static bool LegalizeEnemy(houseType *house, size_t roomNum, size_t objectNum, bool &anyRepairs)
{
	roomType *room = house->rooms + roomNum;
	objectType *obj = room->objects + objectNum;
	enemyType *enemy = &obj->data.h;

	LegalizeBoolean(enemy->initial);
	LegalizeBoolean(enemy->state);

	LegalizeTopLeft(enemy->topLeft, anyRepairs);

	Rect bounds = srcRects[obj->what];
	ZeroRectCorner(&bounds);
	QOffsetRect(&bounds, enemy->topLeft.h, enemy->topLeft.v);

	if (ForceRectInRoomRect(bounds))
	{
		anyRepairs = RCheck(true);
		enemy->topLeft.h = bounds.left;
		enemy->topLeft.v = bounds.top;
	}

	switch (obj->what)
	{
	case kBalloon:
	case kCopterLf:
	case kCopterRt:
		{
			int expectedV = (kTileHigh / 2) - HalfRectTall(&bounds);
			if (enemy->topLeft.v != expectedV)
			{
				anyRepairs = RCheck(true);
				enemy->topLeft.v = expectedV;
			}
		}
		break;
	case kDartLf:
		{
			int expectedH = kRoomWide - RectWide(&bounds);
			if (enemy->topLeft.h != expectedH)
			{
				anyRepairs = RCheck(true);
				enemy->topLeft.h = expectedH;
			}
		}
		break;
	case kDartRt:
		if (enemy->topLeft.h != 0)
		{
			anyRepairs = RCheck(true);
			enemy->topLeft.h = 0;
		}
		break;
	case kBall:
	case kFish:
		{
			int maxLength = bounds.top;
			if (enemy->length < 0)
			{
				anyRepairs = RCheck(true);
				enemy->length = 0;
			}
			else if (enemy->length > maxLength)
			{
				anyRepairs = RCheck(true);
				enemy->length = maxLength;
			}
		}
		break;
	case kDrip:
		{
			int maxLength = kTileHigh - bounds.bottom;
			if (enemy->length < 0)
			{
				anyRepairs = RCheck(true);
				enemy->length = 0;
			}
			else if (enemy->length > maxLength)
			{
				anyRepairs = RCheck(true);
				enemy->length = maxLength;
			}
		}
		break;
	case kCobweb:
		break;
	}

	return true;
}

static bool LegalizeClutter(houseType *house, size_t roomNum, size_t objectNum, bool &anyRepairs)
{
	roomType *room = house->rooms + roomNum;
	objectType *obj = room->objects + objectNum;
	clutterType *blower = &obj->data.i;

	PL_NotYetImplemented_TODO("Validate");
	return true;
}


static bool LegalizeObject(houseType *house, size_t roomNum, size_t objectNum, bool &anyRepairs)
{
	roomType *room = house->rooms + roomNum;
	objectType *obj = room->objects + objectNum;

	switch (obj->what)
	{
	default:
		obj->what = kObjectIsEmpty;
		anyRepairs = RCheck(true);
		break;
	case kObjectIsEmpty:
		break;

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
		return LegalizeBlower(house, roomNum, objectNum, anyRepairs);

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
		return LegalizeFurniture(house, roomNum, objectNum, anyRepairs);

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
		return LegalizeBonus(house, roomNum, objectNum, anyRepairs);

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
		return LegalizeTransport(house, roomNum, objectNum, anyRepairs);

	case kLightSwitch:
	case kMachineSwitch:
	case kThermostat:
	case kPowerSwitch:
	case kKnifeSwitch:
	case kInvisSwitch:
	case kTrigger:
	case kLgTrigger:
	case kSoundTrigger:
		return LegalizeSwitch(house, roomNum, objectNum, anyRepairs);

	case kCeilingLight:
	case kLightBulb:
	case kTableLamp:
	case kHipLamp:
	case kDecoLamp:
	case kFlourescent:
	case kTrackLight:
	case kInvisLight:
		return LegalizeLight(house, roomNum, objectNum, anyRepairs);

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
		return LegalizeAppliance(house, roomNum, objectNum, anyRepairs);

	case kBalloon:
	case kCopterLf:
	case kCopterRt:
	case kDartLf:
	case kDartRt:
	case kBall:
	case kDrip:
	case kFish:
	case kCobweb:
		return LegalizeEnemy(house, roomNum, objectNum, anyRepairs);

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
		return LegalizeClutter(house, roomNum, objectNum, anyRepairs);
	}

	return true;
}

static bool LegalizeRoom(houseType *house, size_t roomNum, bool &anyRepairs)
{
	roomType *room = house->rooms + roomNum;
	if (room->suite == kRoomIsEmpty)
		return true;

	if (!LegalizePascalStr(room->name, anyRepairs))
		return LCheck(false);

	LegalizeBoolean(room->visited);

	for (int i = 0; i < kNumTiles; i++)
	{
		if (room->tiles[i] < 0)
		{
			room->tiles[i] = 0;
			anyRepairs = RCheck(true);
		}
		else if (room->tiles[i] >= kNumTiles)
		{
			room->tiles[i] = kNumTiles - 1;
			anyRepairs = RCheck(true);
		}
	}

	// Enforce object type caps
	enum CapType
	{
		CapType_Candle,
		CapType_Tiki,
		CapType_Coals,
		CapType_Pendulum,
		CapType_RubberBands,
		CapType_Star,
		CapType_DynamicObj,
		CapType_SoundTrigger,
		CapType_UpStairs,
		CapType_DownStairs,
		CapType_Grease,

		CapType_Count,
	};

	int caps[CapType_Count];
	caps[CapType_Candle] = kMaxCandles;
	caps[CapType_Tiki] = kMaxTikis;
	caps[CapType_Coals] = kMaxCoals;
	caps[CapType_Pendulum] = kMaxPendulums;
	caps[CapType_RubberBands] = kMaxRubberBands;
	caps[CapType_Star] = kMaxStars;
	caps[CapType_DynamicObj] = kMaxDynamicObs;
	caps[CapType_SoundTrigger] = kMaxSoundTriggers;
	caps[CapType_UpStairs] = kMaxStairs;
	caps[CapType_DownStairs] = kMaxStairs;
	caps[CapType_Grease] = kMaxGrease;

	int counts[CapType_Count];
	for (int i = 0; i < CapType_Count; i++)
		counts[i] = 0;

	for (size_t i = 0; i < kMaxRoomObs; i++)
	{
		CapType capType = CapType_Count;

		objectType *obj = room->objects + i;
		switch (obj->what)
		{
		case kTaper:
		case kCandle:
		case kStubby:
			capType = CapType_Candle;
			break;
		case kTiki:
			capType = CapType_Tiki;
			break;
		case kBBQ:
			capType = CapType_Coals;
			break;
		case kCuckoo:
			capType = CapType_Pendulum;
			break;
		case kBands:
			capType = CapType_RubberBands;
			break;
		case kStar:
			capType = CapType_Star;
			break;
		case kSparkle:
		case kToaster:
		case kMacPlus:
		case kTV:
		case kCoffee:
		case kOutlet:
		case kVCR:
		case kStereo:
		case kMicrowave:
		case kBalloon:
		case kCopterLf:
		case kCopterRt:
		case kDartLf:
		case kDartRt:
		case kBall:
		case kDrip:
		case kFish:
			capType = CapType_DynamicObj;
			break;
		case kSoundTrigger:
			capType = CapType_SoundTrigger;
			break;
		case kUpStairs:
			capType = CapType_UpStairs;
			break;
		case kDownStairs:
			capType = CapType_DownStairs;
			break;
		case kGreaseLf:
		case kGreaseRt:
			capType = CapType_Grease;
			break;
		default:
			break;
		};

		if (capType != CapType_Count)
		{
			if (counts[capType] == caps[capType])
			{
				obj->what = kObjectIsEmpty;
				anyRepairs = RCheck(true);
			}
			else
				counts[capType]++;
		}
	}

	// Check all objects
	for (size_t i = 0; i < kMaxRoomObs; i++)
	{
		if (!LegalizeObject(house, roomNum, i, anyRepairs))
			return LCheck(false);
	}

	int16_t numObjects = kMaxRoomObs;
	for (size_t i = 0; i < kMaxRoomObs; i++)
	{
		objectType *obj = room->objects + i;

		if (obj->what == kObjectIsEmpty)
			numObjects--;
	}

	if (numObjects != room->numObjects)
	{
		anyRepairs = RCheck(true);
		room->numObjects = numObjects;
	}

	return true;
}

static bool LegalizeHouse(houseType *house, bool &anyRepairs)
{
	size_t nRooms = house->nRooms;

	if (!LegalizeScores(&house->highScores, anyRepairs))
		return false;

	PL_NotYetImplemented_TODO("Validate initial pos");

	// Repair room layout
	for (size_t i = 0; i < nRooms; i++)
	{
		if (!LegalizeRoomLayout(house, i, anyRepairs))
			return LCheck(false);
	}

	// Repair firstRoom
	if (house->firstRoom < 0 || house->firstRoom >= house->nRooms || house->rooms[house->firstRoom].suite == kRoomIsEmpty)
	{
		if (nRooms != 0)
		{
			bool repairedOK = false;
			for (size_t i = 0; i < nRooms; i++)
			{
				if (house->rooms[i].suite == kRoomIsEmpty)
					continue;

				house->firstRoom = static_cast<int16_t>(i);
				repairedOK = true;
				anyRepairs = RCheck(true);
			}

			if (!repairedOK)
				return LCheck(false);
		}
	}

	for (size_t i = 0; i < nRooms; i++)
	{
		if (!LegalizeRoom(house, i, anyRepairs))
			return LCheck(false);
	}

	return true;
}

Boolean ReadHouse (GpIOStream *houseStream, bool untrusted)
{
	long		byteCount;
	PLError_t	theErr;
	short		whichRoom;

	// There should be no padding remaining the house type
	GP_STATIC_ASSERT(sizeof(houseType) - sizeof(roomType) == houseType::kBinaryDataSize + 2);
	
	if (!houseOpen)
	{
		YellowAlert(kYellowUnaccounted, 2);
		return (false);
	}

	byteCount = houseStream->Size();
	
	#ifdef COMPILEDEMO
	if (byteCount != 16526L)
		return (false);
	#endif
	
	if (thisHouse != nil)
		thisHouse.Dispose();

	if (byteCount < houseType::kBinaryDataSize)
	{
		YellowAlert(kYellowHouseDamaged, 1);
		return (false);
	}

	const size_t roomDataSize = static_cast<size_t>(byteCount) - houseType::kBinaryDataSize;
	if (roomDataSize % sizeof(roomType) != 0)
	{
		YellowAlert(kYellowHouseDamaged, 2);
		return (false);
	}

	const size_t roomCountFromDataSize = roomDataSize / sizeof(roomType);

	// GP: Correct for padding
	const size_t alignmentPadding = sizeof(houseType) - sizeof(roomType) - houseType::kBinaryDataSize;
	
	thisHouse = NewHandle(byteCount + alignmentPadding).StaticCast<houseType>();
	if (thisHouse == nil)
	{
		YellowAlert(kYellowNoMemory, 10);
		return(false);
	}
	
	if (!houseStream->SeekStart(0))
	{
		CheckFileError(PLErrors::kIOError, thisHouseName);
		return(false);
	}
	
	const size_t readByteCount = houseStream->Read(*thisHouse, byteCount);
	if (readByteCount != byteCount || readByteCount < houseType::kBinaryDataSize)
	{
		CheckFileError(PLErrors::kIOError, thisHouseName);
		return(false);
	}

	if (alignmentPadding != 0)
	{
		// GP: Correct for padding
		const size_t roomDataSize = byteCount - houseType::kBinaryDataSize;

		uint8_t *houseDataBytes = reinterpret_cast<uint8_t*>(*thisHouse);
		memmove((*thisHouse)->rooms, houseDataBytes + houseType::kBinaryDataSize, roomDataSize);
	}

	ByteSwapHouse(*thisHouse, static_cast<size_t>(byteCount), false);
	
	numberRooms = (*thisHouse)->nRooms;
	if (numberRooms < 0 || static_cast<size_t>(numberRooms) > roomCountFromDataSize)
	{
		YellowAlert(kYellowHouseDamaged, 3);
		return (false);
	}

	bool anyRepairs = false;
	if (untrusted)
	{
		if (!LegalizeHouse(*thisHouse, anyRepairs))
		{
			YellowAlert(kYellowHouseDamaged, 4);
			return (false);
		}

		if (anyRepairs)
			YellowAlert(kYellowHouseRepaired, 0);
	}

	#ifdef COMPILEDEMO
	if (numberRooms != 45)
		return (false);
	#endif
	if ((numberRooms < 1) || (byteCount == 0L))
	{
		numberRooms = 0;
		noRoomAtAll = true;
		YellowAlert(kYellowNoRooms, 0);
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
	PLError_t			theErr;

	if ((housesFound < 1) || (thisHouseIndex == -1))
		return(false);

	if (!houseOpen)
	{
		YellowAlert(kYellowUnaccounted, 4);
		return (false);
	}

	if (!houseCFile)
	{
		YellowAlert(kYellowUnaccounted, 4);
		return (false);
	}

	GpIOStream *houseStream = nil;
	theErr = houseCFile->OpenData(PortabilityLayer::EFilePermission_Write, GpFileCreationDispositions::kCreateOrOverwrite, houseStream);
	if (theErr != PLErrors::kNone)
		return (false);

	CopyThisRoomToRoom();
	
	if (checkIt)
		CheckHouseForProblems();
	
	byteCount = GetHandleSize(thisHouse.StaticCast<void>());
	
	if (fileDirty)
	{
		int64_t currentTime = PLDrivers::GetSystemServices()->GetTime();
		if (currentTime > 0x7fffffff)
			currentTime = 0x7fffffff;

		if (changeLockStateOfHouse)
			houseUnlocked = !saveHouseLocked;
		
		if (houseUnlocked)								// house unlocked
			timeStamp &= 0x7FFFFFFE;
		else
			timeStamp |= 0x00000001;
		(*thisHouse)->timeStamp = (long)timeStamp;
		(*thisHouse)->version = wasHouseVersion;
	}

	long headerSize = houseType::kBinaryDataSize;
	long roomsSize = sizeof(roomType) * (*thisHouse)->nRooms;

	ByteSwapHouse(*thisHouse, static_cast<size_t>(byteCount), true);

	if (houseStream->Write(*thisHouse, headerSize) != headerSize)
	{
		CheckFileError(PLErrors::kIOError, thisHouseName);
		ByteSwapHouse(*thisHouse, static_cast<size_t>(byteCount), false);
		houseStream->Close();
		return(false);
	}

	if (houseStream->Write((*thisHouse)->rooms, roomsSize) != roomsSize)
	{
		CheckFileError(PLErrors::kIOError, thisHouseName);
		ByteSwapHouse(*thisHouse, static_cast<size_t>(byteCount), false);
		houseStream->Close();
		return(false);
	}

	ByteSwapHouse(*thisHouse, static_cast<size_t>(byteCount), false);

	houseStream->Close();
	
	if (changeLockStateOfHouse)
	{
		changeLockStateOfHouse = false;
		ReflectCurrentRoom(true);
	}
	
	fileDirty = false;
	UpdateMenus(false);
	return (true);
}

//--------------------------------------------------------------  CloseHouse
// This function closes the current house that is open.

Boolean CloseHouse (void)
{
	PLError_t		theErr;
	
	if (!houseOpen)
		return (true);
	
	if (fileDirty)
	{
#ifndef COMPILEDEMO
		if (!QuerySaveChanges())	// false signifies user canceled
			return(false);
#endif
	}
	
	CloseHouseResFork();
	CloseHouseMovie();

	if (houseCFile)
	{
		houseCFile->Close();
		houseCFile = nil;
	}
	
	houseOpen = false;
	
	return (true);
}

//--------------------------------------------------------------  OpenHouseResFork
// Opens the resource fork of the current house that is open.

void OpenHouseResFork (void)
{
	PortabilityLayer::ResourceManager *rm = PortabilityLayer::ResourceManager::GetInstance();
	if (houseResFork == nullptr)
	{
		houseResFork = rm->LoadResFile(houseCFile);
		if (!houseResFork)
			YellowAlert(kYellowFailedResOpen, PLErrors::kResourceError);
	}
}

//--------------------------------------------------------------  CloseHouseResFork
// Closes the resource fork of the current house that is open.

void CloseHouseResFork (void)
{
	if (houseResFork)
	{
		PortabilityLayer::ResourceManager *rm = PortabilityLayer::ResourceManager::GetInstance();

		houseResFork->Destroy();
		houseResFork = nullptr;
	}
}

//--------------------------------------------------------------  QuerySaveChanges
// If changes were made, this function will present the user with aÉ
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
	DialogTextSubstitutions substitutions(thisHouseName);
	hitWhat = PortabilityLayer::DialogManager::GetInstance()->DisplayAlert(kSaveChangesAlert, &substitutions);
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
			OpenHouse(true);
		}
		UpdateMenus(false);
		return (true);
	}
	else
		return (false);
}
#endif

//--------------------------------------------------------------  YellowAlert
// This is a dialog used to present an error code and explanationÉ
// to the user when a non-lethal error has occurred.  Ideally, ofÉ
// course, this never is called.

void YellowAlert (short whichAlert, short identifier)
{
	IGpLogDriver *logger = PLDrivers::GetLogDriver();

	if (logger)
		logger->Printf(IGpLogDriver::Category_Warning, "Yellow alert %i identifier %i", static_cast<int>(whichAlert), static_cast<int>(identifier));

	#define		kYellowAlert	1006
	Str255		errStr, errNumStr;
	short		whoCares;
	
	InitCursor();
	
	GetIndString(errStr, kYellowAlert, whichAlert);
	NumToString((long)identifier, errNumStr);
	
//	CenterAlert(kYellowAlert);
	DialogTextSubstitutions substitutions(errStr, errNumStr);
	
	whoCares = PortabilityLayer::DialogManager::GetInstance()->DisplayAlert(kYellowAlert, &substitutions);
}

//--------------------------------------------------------------  LoadHousePicture

THandle<void> LoadHouseResource(const PortabilityLayer::ResTypeID &resTypeID, int16_t resID)
{
	THandle<void> hdl = houseResFork->LoadResource(resTypeID, resID);
	if (hdl != nullptr)
		return hdl;

	return PortabilityLayer::ResourceManager::GetInstance()->GetAppResource(resTypeID, resID);
}

//--------------------------------------------------------------  ExportHouse

namespace ExportHouseResults
{
	enum ExportHouseResult
	{
		kOK,

		kStreamFailed,
		kIOError,
		kMemError,
		kResourceError,
		kInternalError,
	};
}

typedef ExportHouseResults::ExportHouseResult ExportHouseResult_t;

struct SimpleResource
{
	PortabilityLayer::ResTypeID m_resType;
	int m_resourceID;
	PortabilityLayer::PascalStr<255> m_name;
	size_t m_offsetInResData;
	uint8_t m_attributes;
};

static bool AppendRaw(GpVector<uint8_t> &bytes, const void *data, size_t size)
{
	for (size_t i = 0; i < size; i++)
	{
		if (!bytes.Append(static_cast<const uint8_t*>(data)[i]))
			return false;
	}

	return true;
}

template<class T>
static bool AppendRawStruct(GpVector<uint8_t> &bytes, const T &value)
{
	return AppendRaw(bytes, &value, sizeof(T));
}

static ExportHouseResult_t TryExportSound(GpVector<uint8_t> &resData, const THandle<void> &resHandle)
{
	const void *dataContents = nullptr;
	size_t dataSize = 0;
	if (!ParseAndConvertSoundChecked(resHandle, dataContents, dataSize))
		return ExportHouseResults::kResourceError;

	// Don't ask...
	const uint8_t commandStreamPrefix[20] = { 0, 1, 0, 1, 0, 5, 0, 0, 0, 0xa0, 0, 1, 0x80, 0x51, 0, 0, 0, 0, 0, 0x14 };

	struct BufferHeader
	{
		BEUInt32_t m_samplePtr;
		BEUInt32_t m_length;
		BEFixed32_t m_sampleRate;
		BEUInt32_t m_loopStart;
		BEUInt32_t m_loopEnd;
		uint8_t m_encoding;
		uint8_t m_baseFrequency;
	};

	BufferHeader bufferHeader;
	bufferHeader.m_samplePtr = 0;
	bufferHeader.m_length = static_cast<uint32_t>(dataSize);
	bufferHeader.m_sampleRate.m_intPart = 0x56ee;
	bufferHeader.m_sampleRate.m_fracPart = 0x8ba3;
	bufferHeader.m_loopStart = static_cast<uint32_t>(dataSize - 2);
	bufferHeader.m_loopEnd = static_cast<uint32_t>(dataSize - 1);
	bufferHeader.m_encoding = 0;
	bufferHeader.m_baseFrequency = 0x3c;

	if (!resData.Resize(sizeof(bufferHeader) + sizeof(commandStreamPrefix) + dataSize))
		return ExportHouseResults::kMemError;

	memcpy(&resData[0], commandStreamPrefix, sizeof(commandStreamPrefix));
	memcpy(&resData[sizeof(commandStreamPrefix)], &bufferHeader, sizeof(bufferHeader));

	if (dataSize > 0)
		memcpy(&resData[sizeof(commandStreamPrefix) + sizeof(bufferHeader)], dataContents, dataSize);

	return ExportHouseResults::kOK;
}

static void BitSwap4(GpVector<uint8_t> &vec, size_t count)
{
	for (size_t i = 0; i < count; i++)
	{
		uint8_t v = vec[i];
		v = (((v >> 4) & 0xf) | ((v << 4) & 0xf));
		vec[i] = v;
	}
}

static void BitSwap2(GpVector<uint8_t> &vec, size_t count)
{
	for (size_t i = 0; i < count; i++)
	{
		uint8_t v = vec[i];
		v = (((v >> 2) & 0x33) | ((v << 2) & 0xcc));
		vec[i] = v;
	}
}

static void BitSwap1(GpVector<uint8_t> &vec, size_t count)
{
	for (size_t i = 0; i < count; i++)
	{
		uint8_t v = vec[i];
		v = (((v >> 1) & 0x55) | ((v << 1) & 0xaa));
		vec[i] = v;
	}
}

namespace RLEEncoder
{
	static const size_t kMaxRepeat = 129;
	static const size_t kMaxLiteral = 128;

	bool EmitSymbol(GpVector<uint8_t> &compressedData, uint8_t sym)
	{
		return compressedData.Append(sym);
	}

	bool EmitSymbol(GpVector<uint8_t> &compressedData, uint16_t sym)
	{
		return compressedData.Append((sym >> 8) & 0xff) && compressedData.Append(sym & 0xff);
	}

	template<class T>
	bool EmitLiterals(GpVector<uint8_t> &compressedData, const T *symbols, size_t length)
	{
		if (length == 0)
			return true;

		assert(length <= kMaxLiteral);
		if (!compressedData.Append(static_cast<uint8_t>(length - 1)))
			return false;

		for (size_t i = 0; i < length; i++)
		{
			if (!EmitSymbol(compressedData, symbols[i]))
				return false;
		}

		return true;
	}

	template<class T>
	bool EmitRepeat(GpVector<uint8_t> &compressedData, const T &symbol, size_t length)
	{
		if (length < 2)
			return EmitLiterals(compressedData, &symbol, length);

		assert(length <= kMaxRepeat);
		if (!compressedData.Append(static_cast<uint8_t>(257 - length)))
			return false;

		if (!EmitSymbol(compressedData, symbol))
			return false;
	
		return true;
	}


	template<class T>
	bool PackRLE(GpVector<uint8_t> &compressedData, const GpVector<T> &uncompressedData)
	{
		size_t numUncompressed = uncompressedData.Count();
		const T *uncompressedSymbols = uncompressedData.Buffer();

		if (!compressedData.Resize(0))
			return false;

		size_t literalStartLoc = 0;
		size_t repeatStartLoc = 0;
		size_t readPos = 0;

		// Loop/exit invariants:
		// repeatStartLoc - literalStartLoc <= kMaxLiteral
		// i - repeatStartLoc < kMaxRepeat
		for (size_t i = 0; i < numUncompressed; i++)
		{
			T b = uncompressedSymbols[i];
			if (b != uncompressedSymbols[repeatStartLoc])
			{
				// Run terminates at i
				const size_t repeatLength = i - repeatStartLoc;
				const size_t literalLength = repeatStartLoc - literalStartLoc;

				// Determine if we should flush the repeat or fold it into the literal span.
				// There are several situations that can happen here:
				// Repeat length is 1:
				//   Literal span is at limit:
				//     Emit literal span, start new literal span at repeatStartLoc
				//   Literal span is below limit:
				//     Do nothing
				// Repeat length is 2:
				//   Literal span is 0:
				//     Emit repeat
				//   Literal span is non-zero and appending repeat to literal span would be at or exceed limit:
				//     Emit literal span and repeat
				//   Otherwise:
				//     Do nothing
				// Repeat length is 3+:
				//   Emit literal span and repeat
				if (repeatLength == 1)
				{
					if (literalLength == kMaxLiteral)
					{
						if (!EmitLiterals(compressedData, uncompressedSymbols + literalStartLoc, literalLength))
							return false;

						literalStartLoc = repeatStartLoc;
					}
				}
				else if (repeatLength == 2)
				{
					if (literalLength == 0 || (literalLength + repeatLength >= kMaxLiteral))
					{
						if (literalLength != 0)
						{
							if (!EmitLiterals(compressedData, uncompressedSymbols + literalStartLoc, literalLength))
								return false;
						}

						if (!EmitRepeat(compressedData, uncompressedSymbols[repeatStartLoc], repeatLength))
							return false;

						literalStartLoc = i;
					}
				}
				else if (repeatLength >= 3)
				{
					if (literalLength != 0)
					{
						if (!EmitLiterals(compressedData, uncompressedSymbols + literalStartLoc, literalLength))
							return false;
					}

					if (!EmitRepeat(compressedData, uncompressedSymbols[repeatStartLoc], repeatLength))
						return false;

					literalStartLoc = i;
				}

				repeatStartLoc = i;
			}
			else
			{
				// i is a repeat character
				const size_t repeatLength = i + 1 - repeatStartLoc;
				const size_t literalLength = repeatStartLoc - literalStartLoc;

				if (repeatLength == kMaxRepeat)
				{
					if (literalLength != 0)
					{
						if (!EmitLiterals(compressedData, uncompressedSymbols + literalStartLoc, literalLength))
							return false;
					}

					if (!EmitRepeat(compressedData, uncompressedSymbols[repeatStartLoc], repeatLength))
						return false;

					literalStartLoc = i + 1;
					repeatStartLoc = i + 1;
				}
			}
		}

		// Final flush
		size_t repeatLength = numUncompressed - repeatStartLoc;
		size_t literalLength = repeatStartLoc - literalStartLoc;

		if (repeatLength == 1)
		{
			if (literalLength < kMaxLiteral)
			{
				literalLength++;
				repeatLength = 0;
			}
		}

		if (literalLength != 0)
		{
			if (!EmitLiterals(compressedData, uncompressedSymbols + literalStartLoc, literalLength))
				return false;
		}

		if (repeatLength == 1)
		{
			if (!EmitLiterals(compressedData, uncompressedSymbols + repeatStartLoc, 1))
				return false;
		}
		else if (repeatLength >= 2)
		{
			if (!EmitRepeat(compressedData, uncompressedSymbols[repeatStartLoc], repeatLength))
				return false;
		}

		return true;
	}
}

static ExportHouseResult_t TryExportPictFromSurface(GpVector<uint8_t> &resData, DrawSurface *surface)
{
	bool couldBe16Bit = true;
	bool couldBe8Bit = true;
	int numUniqueColors = 0;
	PortabilityLayer::RGBAColor uniqueColors[256];
	for (int i = 0; i < 256; i++)
		uniqueColors[i] = PortabilityLayer::RGBAColor::Create(0, 0, 0, 255);

	const Rect rect = surface->m_port.GetRect();

	const size_t width = rect.Width();
	const size_t height = rect.Height();

	THandle<PixMap> pixMapHdl = surface->m_port.GetPixMap();
	const PixMap *pixMap = *pixMapHdl;
	const uint8_t *imageData = static_cast<const uint8_t*>(pixMap->m_data);
	const size_t pixelDataPitch = pixMap->m_pitch;

	assert(pixMap->m_pixelFormat == GpPixelFormats::kRGB32);
	

	for (size_t row = 0; row < height; row++)
	{
		const uint8_t *rowData = imageData + pixMap->m_pitch * row;

		if (!couldBe8Bit && !couldBe16Bit)
			break;

		for (size_t col = 0; col < width; col++)
		{
			const uint8_t *pixelData = rowData + col * 4;
			if (couldBe16Bit)
			{
				for (int ch = 0; ch < 3; ch++)
				{
					uint8_t channelData = pixelData[ch];
					if ((channelData >> 2) != (channelData & 0x7))
					{
						couldBe16Bit = false;
						break;
					}
				}
			}

			if (couldBe8Bit)
			{
				PortabilityLayer::RGBAColor rgbaColor = PortabilityLayer::RGBAColor::Create(pixelData[0], pixelData[1], pixelData[2], pixelData[3]);

				bool matchedColor = false;
				for (int i = 0; i < numUniqueColors; i++)
				{
					if (uniqueColors[i] == rgbaColor)
					{
						matchedColor = true;
						break;
					}
				}

				if (!matchedColor)
				{
					if (numUniqueColors == 256)
						couldBe8Bit = false;
					else
						uniqueColors[numUniqueColors++] = rgbaColor;
				}
			}
		}
	}

	bool isBWBitmap = false;

	if (numUniqueColors <= 2)
	{
		isBWBitmap = true;
		for (int c = 0; c < numUniqueColors; c++)
		{
			if (uniqueColors[c] != StdColors::Black() && uniqueColors[c] != StdColors::White())
				isBWBitmap = false;
		}

		if (isBWBitmap)
		{
			numUniqueColors = 2;
			uniqueColors[0] = StdColors::White();
			uniqueColors[1] = StdColors::Black();
		}
	}

	int bpp = 0;
	if (isBWBitmap)
		bpp = 1;
	else if (couldBe8Bit)
	{
		if (numUniqueColors <= 2)
			bpp = 1;
		else if (numUniqueColors <= 4)
			bpp = 2;
		else if (numUniqueColors <= 16)
			bpp = 4;
		else
			bpp = 8;
	}
	else if (couldBe16Bit)
		bpp = 16;
	else
		bpp = 32;

	// The typical structure of a PICT is header, ClipRegion, then a raster op.
	// We use V1 pict for 1bpp and V2 for all others.
	struct PictHeader
	{
		uint8_t m_size[2];

		BERect m_rect;
	};

	BERect beRect;
	beRect.top = beRect.left = 0;
	beRect.right = static_cast<int16_t>(width);
	beRect.bottom = static_cast<int16_t>(height);

	PictHeader pictHeader;
	pictHeader.m_size[0] = 0;
	pictHeader.m_size[1] = 0;
	pictHeader.m_rect = beRect;

	if (!AppendRawStruct(resData, pictHeader))
		return ExportHouseResults::kMemError;

	int pictVersion = 1;
	if (isBWBitmap)
	{
		const uint8_t versionTag[2] = { 0x11, 0x01 };
		if (!AppendRaw(resData, versionTag, 2))
			return ExportHouseResults::kMemError;
	}
	else
	{
		pictVersion = 2;

		struct PictV2Header
		{
			BEUInt16_t m_versionTag;
			BEUInt16_t m_versionOp;
			BEUInt16_t m_headerOp;
			BEInt16_t m_v2Version;
			BEInt16_t m_reserved1;
			BEFixed32_t m_top;
			BEFixed32_t m_left;
			BEFixed32_t m_bottom;
			BEFixed32_t m_right;
			BEUInt32_t m_reserved2;
		};

		GP_STATIC_ASSERT(sizeof(PictV2Header) == 30);

		PictV2Header v2Header;
		v2Header.m_versionTag = 0x0011;
		v2Header.m_versionOp = 0x02ff;
		v2Header.m_headerOp = 0x0c00;
		v2Header.m_v2Version = -1;
		v2Header.m_reserved1 = -1;
		v2Header.m_top.m_intPart = beRect.top;
		v2Header.m_top.m_fracPart = 0;
		v2Header.m_left.m_intPart = beRect.left;
		v2Header.m_left.m_fracPart = 0;
		v2Header.m_bottom.m_intPart = beRect.bottom;
		v2Header.m_bottom.m_fracPart = 0;
		v2Header.m_right.m_intPart = beRect.right;
		v2Header.m_right.m_fracPart = 0;

		if (!AppendRawStruct(resData, v2Header))
			return ExportHouseResults::kMemError;
	}

	// Emit ClipRgn opcode
	if (pictVersion == 1)
	{
		const uint8_t clipRgnOpcode[1] = { PortabilityLayer::QDOpcodes::kClipRegion };
		if (!AppendRaw(resData, clipRgnOpcode, 1))
			return ExportHouseResults::kMemError;
	}
	else if (pictVersion == 2)
	{
		const uint8_t clipRgnOpcode[2] = { 0, PortabilityLayer::QDOpcodes::kClipRegion };
		if (!AppendRaw(resData, clipRgnOpcode, 2))
			return ExportHouseResults::kMemError;
	}

	struct ClipRgnData
	{
		BEUInt16_t m_structureSize;
		BERect m_rect;
	};

	GP_STATIC_ASSERT(sizeof(ClipRgnData) == 10);

	ClipRgnData clipRgnData;
	clipRgnData.m_structureSize = sizeof(ClipRgnData);
	clipRgnData.m_rect = beRect;

	if (!AppendRawStruct(resData, clipRgnData))
		return ExportHouseResults::kMemError;

	// Emit image
	const size_t bitsPerRow = width * bpp;
	const size_t bytesPerRow = (bitsPerRow + 7) / 8;
	const size_t expansionCapacity = bytesPerRow + (bytesPerRow / 128) + 16;	// Worst-case scenario for RLE failure to compress

	uint16_t bitmapOpcode = 0;
	int packType = 0;
	bool isDirect = false;
	if (bpp <= 8)
	{
		if (bytesPerRow < 8)
		{
			packType = 1;
			bitmapOpcode = PortabilityLayer::QDOpcodes::kBitsRect;
		}
		else
		{
			packType = 0;
			bitmapOpcode = PortabilityLayer::QDOpcodes::kPackBitsRect;
		}
	}
	else
	{
		isDirect = true;
		bitmapOpcode = PortabilityLayer::QDOpcodes::kDirectBitsRect;

		if (bpp == 16)
			packType = 3;
		else if (bpp == 32)
			packType = 4;
		else
			return ExportHouseResults::kInternalError;
	}

	if (pictVersion == 1)
	{
		const uint8_t opcode = bitmapOpcode;
		if (!AppendRaw(resData, &opcode, 1))
			return ExportHouseResults::kMemError;
	}
	else if (pictVersion == 2)
	{
		const BEUInt16_t opcode(bitmapOpcode);
		if (!AppendRaw(resData, &opcode, 2))
			return ExportHouseResults::kMemError;
	}

	// Write prelude
	const bool isPixmap = !isBWBitmap;
	{
		if (isDirect)
		{
			struct DirectPrelude
			{
				BEUInt32_t m_baseAddress;
				BEUInt16_t m_rowSize;
			};

			DirectPrelude prelude;
			prelude.m_baseAddress = 0;
			prelude.m_rowSize = static_cast<uint16_t>(0x8000 | bytesPerRow);

			if (!AppendRawStruct(resData, prelude))
				return ExportHouseResults::kMemError;
		}
		else
		{
			uint16_t rowSize = bytesPerRow;
			if (!isBWBitmap)
				rowSize |= 0x8000;

			BEUInt16_t rowSizeBE(rowSize);

			if (!AppendRawStruct(resData, rowSizeBE))
				return ExportHouseResults::kMemError;
		}
	}

	// Do actual image packing, we need to do this now so we have the pack size available
	GpVector<uint8_t> packedImage(PLDrivers::GetAlloc());

	GpVector<uint8_t> uncompressedRowData8(PLDrivers::GetAlloc());
	GpVector<uint16_t> uncompressedRowData16(PLDrivers::GetAlloc());
	GpVector<uint8_t> compressedRowData(PLDrivers::GetAlloc());

	if (packType == 4)
	{
		if (!uncompressedRowData8.Resize(bytesPerRow))
			return ExportHouseResults::kMemError;
	}
	else
	{
		if (!uncompressedRowData16.Resize(bytesPerRow / 2))
			return ExportHouseResults::kMemError;
	}

	if (!compressedRowData.Resize(expansionCapacity))
		return ExportHouseResults::kMemError;

	for (size_t row = 0; row < height; row++)
	{
		if (!uncompressedRowData8.Resize(0))
			return ExportHouseResults::kMemError;

		if (!uncompressedRowData16.Resize(0))
			return ExportHouseResults::kMemError;

		const uint8_t *srcRowStart = imageData + pixelDataPitch * row;

		if (packType == 4)
		{
			// RGB24 images
			for (size_t ch = 0; ch < 3; ch++)
			{
				for (size_t col = 0; col < width; col++)
				{
					const uint8_t *srcPixelStart = srcRowStart + col * 4;
					if (!uncompressedRowData8.Append(srcPixelStart[ch]))
						return ExportHouseResults::kMemError;
				}
			}
		}
		else if (couldBe16Bit)
		{
			assert(packType == 3);
			for (size_t col = 0; col < width; col++)
			{
				const uint8_t *srcPixelStart = srcRowStart + col * 4;
				uint16_t packed = ((srcPixelStart[0] << 7) & 0x7c00) | ((srcPixelStart[1] << 2) & 0x3e0) | ((srcPixelStart[2] >> 3) & 0x1f);
				if (!uncompressedRowData16.Append(packed))
					return ExportHouseResults::kMemError;
			}
		}
		else
		{
			assert(couldBe8Bit);

			int numBitsSpooled = 0;
			uint8_t spooledBits = 0;
			for (size_t col = 0; col < width; col++)
			{
				const uint8_t *srcPixelStart = srcRowStart + col * 4;
				PortabilityLayer::RGBAColor color = PortabilityLayer::RGBAColor::Create(srcPixelStart[0], srcPixelStart[1], srcPixelStart[2], srcPixelStart[3]);
				int colorIndex = -1;
				for (int ci = 0; ci < numUniqueColors; ci++)
				{
					if (color == uniqueColors[ci])
					{
						colorIndex = ci;
						break;
					}
				}

				assert(colorIndex >= 0);

				spooledBits <<= bpp;
				spooledBits |= colorIndex;
				numBitsSpooled += bpp;

				if (numBitsSpooled == 8)
				{
					if (!uncompressedRowData8.Append(spooledBits))
						return ExportHouseResults::kMemError;

					numBitsSpooled = 0;
					spooledBits = 0;
				}
			}

			if (numBitsSpooled != 0)
			{
				spooledBits <<= (8 - numBitsSpooled);
				if (!uncompressedRowData8.Append(spooledBits))
					return ExportHouseResults::kMemError;
			}
		}

		if (!compressedRowData.Resize(0))
			return ExportHouseResults::kMemError;

		bool needsLengthMarker = false;
		switch (packType)
		{
		case 0:
		case 4:
			// 8-bit RLE
			if (!RLEEncoder::PackRLE<uint8_t>(compressedRowData, uncompressedRowData8))
				return ExportHouseResults::kMemError;

			needsLengthMarker = true;
			break;
		case 1:
			// Uncompressed
			if (!compressedRowData.Resize(bytesPerRow))
				return ExportHouseResults::kMemError;

			for (size_t i = 0; i < bytesPerRow; i++)
				compressedRowData[i] = uncompressedRowData8[i];

			needsLengthMarker = false;
			break;
		case 3:
			// 16-bit RLE
			if (!RLEEncoder::PackRLE<uint16_t>(compressedRowData, uncompressedRowData16))
				return ExportHouseResults::kMemError;
			needsLengthMarker = true;
			break;
		default:
			assert(false);
			return ExportHouseResults::kInternalError;
		};

		const size_t compressedSize = compressedRowData.Count();
		if (needsLengthMarker)
		{
			if (bytesPerRow > 250)
				packedImage.Append((compressedSize >> 8) & 0xff);
			packedImage.Append(compressedSize & 0xff);
		}

		for (size_t i = 0; i < compressedSize; i++)
			packedImage.Append(compressedRowData[i]);
	}


	// Write BitMap/PixMap
	{
		if (isBWBitmap)
		{
			struct BitMapData
			{
				BEBitMap m_bitMap;
				BERect m_srcRect;
				BERect m_destRect;
				BEUInt16_t m_transferMode;
			};

			BitMapData bmData;
			bmData.m_bitMap.m_bounds = beRect;
			bmData.m_srcRect = beRect;
			bmData.m_destRect = beRect;
			bmData.m_transferMode = 0;

			if (!AppendRawStruct(resData, bmData))
				return ExportHouseResults::kMemError;
		}
		else
		{
			BEPixMap pixMap;
			pixMap.m_bounds = beRect;
			pixMap.m_version = 0;
			pixMap.m_packType = packType;
			pixMap.m_packSize = static_cast<uint32_t>(packedImage.Count());
			pixMap.m_hRes = 0x480000;
			pixMap.m_vRes = 0x480000;
			pixMap.m_pixelType = (isDirect ? 16 : 0);
			pixMap.m_pixelSize = bpp;
			pixMap.m_componentCount = isDirect ? 3 : 1;

			if (bpp == 32)
				pixMap.m_componentSize = 8;
			else if (bpp == 16)
				pixMap.m_componentSize = 5;
			else
				pixMap.m_componentSize = bpp;

			pixMap.m_planeSizeBytes = 0;
			pixMap.m_clutHandle = 0;
			pixMap.m_unused = 0;

			if (!AppendRawStruct(resData, pixMap))
				return ExportHouseResults::kMemError;

			if (isDirect)
			{
			}
			else
			{
				BEColorTableHeader clutHeader;
				clutHeader.m_resourceID = 0;
				clutHeader.m_flags = 0;
				clutHeader.m_numItemsMinusOne = numUniqueColors - 1;

				if (!AppendRawStruct(resData, clutHeader))
					return ExportHouseResults::kMemError;

				for (int i = 0; i < numUniqueColors; i++)
				{
					BEColorTableItem item;
					item.m_index = i;
					item.m_red[0] = item.m_red[1] = uniqueColors[i].r;
					item.m_green[0] = item.m_green[1] = uniqueColors[i].g;
					item.m_blue[0] = item.m_blue[1] = uniqueColors[i].b;

					if (!AppendRawStruct(resData, item))
						return ExportHouseResults::kMemError;
				}
			}

			struct TransferData
			{
				BERect m_srcRect;
				BERect m_destRect;
				BEUInt16_t m_transferMode;
			};

			TransferData transferData;
			transferData.m_srcRect = beRect;
			transferData.m_destRect = beRect;
			transferData.m_transferMode = 0;

			if (!AppendRawStruct(resData, transferData))
				return ExportHouseResults::kMemError;
		}
	}

	// Write image contents
	if (!AppendRaw(resData, &packedImage[0], packedImage.Count()))
		return ExportHouseResults::kMemError;

	// Write pad byte (??)
	if (!isBWBitmap && (resData.Count() & 1) != 0)
	{
		if (!resData.Append(0))
			return ExportHouseResults::kMemError;
	}

	// Emit EOP opcode
	if (pictVersion == 1)
	{
		const uint8_t eopOpcode[1] = { PortabilityLayer::QDOpcodes::kEndOfPicture };
		if (!AppendRaw(resData, eopOpcode, 1))
			return ExportHouseResults::kMemError;
	}
	else if (pictVersion == 2)
	{
		const uint8_t eopOpcode[2] = { 0, PortabilityLayer::QDOpcodes::kEndOfPicture };
		if (!AppendRaw(resData, eopOpcode, 2))
			return ExportHouseResults::kMemError;
	}

	return ExportHouseResults::kOK;
}

static ExportHouseResult_t TryExportPICT(GpVector<uint8_t> &resData, const THandle<void> &resHandle)
{
	// Parse bitmap header
	const THandle<BitmapImage> bmpHandle = resHandle.StaticCast<BitmapImage>();
	const BitmapImage *bmp = *bmpHandle;

	const Rect rect = bmp->GetRect();
	DrawSurface *surface = nullptr;
	if (NewGWorld(&surface, GpPixelFormats::kRGB32, &rect, nullptr) != PLErrors::kNone)
		return ExportHouseResults::kMemError;

	surface->DrawPicture(bmpHandle, rect, false);

	ExportHouseResult_t result = TryExportPictFromSurface(resData, surface);

	DisposeGWorld(surface);

	return result;
}

static ExportHouseResult_t TryExportResource(GpVector<uint8_t> &resData, PortabilityLayer::IResourceArchive *resArchive, const PortabilityLayer::ResTypeID &resTypeID, int16_t resID)
{
	THandle<void> resHandle = resArchive->LoadResource(resTypeID, resID);
	if (!resHandle)
		return ExportHouseResults::kMemError;

	if (resTypeID == PortabilityLayer::ResTypeID('PICT'))
	{
		ExportHouseResult_t exportResult = TryExportPICT(resData, resHandle);
		resHandle.Dispose();
		return exportResult;
	}

	if (resTypeID == PortabilityLayer::ResTypeID('snd '))
	{
		ExportHouseResult_t exportResult = TryExportSound(resData, resHandle);
		resHandle.Dispose();
		return exportResult;
	}

	const size_t size = resHandle.MMBlock()->m_size;
	if (!resData.Resize(size))
	{
		resHandle.Dispose();
		return ExportHouseResults::kMemError;
	}

	if (size > 0)
		memcpy(&resData[0], *resHandle, size);

	resHandle.Dispose();

	return ExportHouseResults::kOK;
}

ExportHouseResult_t TryExportResources(GpIOStream *stream, PortabilityLayer::IResourceArchive *resArchive)
{
	if (!resArchive)
		return ExportHouseResults::kOK;

	IGpAllocator *alloc = PLDrivers::GetAlloc();

	GpVector<SimpleResource> resources(alloc);

	GpUFilePos_t resForkStart = stream->Tell();

	PortabilityLayer::IResourceIterator *iterator = resArchive->EnumerateResources();
	if (!iterator)
		return ExportHouseResults::kMemError;

	const GpUFilePos_t resForkHeaderPos = stream->Tell();

	GpUFilePos_t resForkDataStart = 0;

	PortabilityLayer::ResTypeID resTypeID;
	int16_t resID = 0;
	bool isFirstResource = true;
	while (iterator->GetOne(resTypeID, resID))
	{
		if (resTypeID != PortabilityLayer::ResTypeID('PICT') && resTypeID != PortabilityLayer::ResTypeID('snd '))
			continue;

		if (isFirstResource)
		{
			// Seems to want this much scratch space...
			uint8_t headerData[256];
			memset(headerData, 0, sizeof(headerData));
			if (!stream->WriteExact(headerData, sizeof(headerData)))
				return ExportHouseResults::kIOError;

			resForkDataStart = stream->Tell();
			isFirstResource = false;
		}

		SimpleResource res;

		bool isPurgeable = true;

		res.m_name.Set(0, nullptr);
		res.m_resourceID = resID;
		res.m_resType = resTypeID;
		res.m_attributes = 0;
		res.m_offsetInResData = stream->Tell() - resForkDataStart;

		if (isPurgeable)
			res.m_attributes |= (1 << 5);

		if (!resources.Append(static_cast<SimpleResource&&>(res)))
		{
			iterator->Destroy();
			return ExportHouseResults::kMemError;
		}

		GpVector<uint8_t> resData(alloc);
		ExportHouseResult_t exportResResult = TryExportResource(resData, resArchive, resTypeID, resID);
		if (exportResResult != ExportHouseResults::kOK)
			return exportResResult;

		BEUInt32_t packedLen(static_cast<uint32_t>(resData.Count()));

		if (!stream->WriteExact(&packedLen, sizeof(packedLen)))
		{
			iterator->Destroy();
			return ExportHouseResults::kIOError;
		}

		if (resData.Count() > 0)
		{
			if (!stream->WriteExact(&resData[0], resData.Count()))
			{
				iterator->Destroy();
				return ExportHouseResults::kIOError;
			}
		}

		const unsigned int unpaddedExcess = ((stream->Tell() - resForkStart) & 0x3);
		if (unpaddedExcess > 0)
		{
			uint8_t padding[4] = { 0, 0, 0, 0 };
			if (!stream->WriteExact(padding, 4 - unpaddedExcess))
				return ExportHouseResults::kIOError;
		}
	}

	iterator->Destroy();

	if (!resources.Count())
		return ExportHouseResults::kOK;

	GpVector<PortabilityLayer::ResTypeID> uniqueResTypes(alloc);
	GpVector<unsigned int> resTypeCounts(alloc);

	// Generate res map
	for (size_t i = 0; i < resources.Count(); i++)
	{
		const SimpleResource &res = resources[i];

		size_t uniqueResTypeIndex = uniqueResTypes.Count();

		for (size_t uri = 0; uri < uniqueResTypes.Count(); uri++)
		{
			if (uniqueResTypes[uri] == res.m_resType)
			{
				uniqueResTypeIndex = uri;
				break;
			}
		}

		if (uniqueResTypeIndex == uniqueResTypes.Count())
		{
			if (!uniqueResTypes.Append(res.m_resType))
				return ExportHouseResults::kMemError;

			if (!resTypeCounts.Append(1))
				return ExportHouseResults::kMemError;
		}
		else
			resTypeCounts[uniqueResTypeIndex]++;
	}

	const GpUFilePos_t resMapPos = stream->Tell();
	const GpUFilePos_t resDataSize = resMapPos - resForkDataStart;

	// Reserved space for resource header copy (16), handle to next res map (4), file ref number (2)
	{
		char resHeaderCopy[22];
		memset(resHeaderCopy, 0, sizeof(resHeaderCopy));

		if (!stream->WriteExact(resHeaderCopy, sizeof(resHeaderCopy)))
			return ExportHouseResults::kIOError;
	}

	uint16_t resForkAttributes = 0;	// We don't use any of these

	const size_t typeListEntrySize = 8;
	const size_t refListEntrySize = 12;

	const size_t resourceTypeListStartLoc = 28;
	const size_t resourceTypeListSize = 2 + uniqueResTypes.Count() * typeListEntrySize;
	const size_t resourceRefListStartLoc = resourceTypeListStartLoc + resourceTypeListSize;
	const size_t resourceNameListStartLoc = resourceRefListStartLoc + resources.Count() * refListEntrySize;

	struct ResForkHeaderData
	{
		BEUInt16_t m_attributes;
		BEUInt16_t m_resourceTypeListStartLoc;
		BEUInt16_t m_resourceNameListStartLoc;
		BEUInt16_t m_numResTypesMinusOne;
	};

	ResForkHeaderData headerData;
	headerData.m_attributes = resForkAttributes;
	headerData.m_resourceTypeListStartLoc = resourceTypeListStartLoc;
	headerData.m_resourceNameListStartLoc = resourceNameListStartLoc;
	headerData.m_numResTypesMinusOne = uniqueResTypes.Count() - 1;

	if (!stream->WriteExact(&headerData, sizeof(headerData)))
		return ExportHouseResults::kIOError;

	GpVector<size_t> refListStartForType(alloc);
	if (!refListStartForType.Resize(resTypeCounts.Count()))
		return ExportHouseResults::kMemError;

	if (resTypeCounts.Count() > 0)
	{
		refListStartForType[0] = 0;
		for (size_t i = 1; i < refListStartForType.Count(); i++)
			refListStartForType[i] = refListStartForType[i - 1] + resTypeCounts[i - 1];
	}

	struct ResTypeData
	{
		char m_resType[4];
		BEUInt16_t m_resCountMinusOne;
		BEUInt16_t m_refListStart;
	};

	// Write resource type list
	for (size_t i = 0; i < uniqueResTypes.Count(); i++)
	{
		ResTypeData resTypeData;
		uniqueResTypes[i].ExportAsChars(resTypeData.m_resType);
		resTypeData.m_resCountMinusOne = resTypeCounts[i] - 1;
		resTypeData.m_refListStart = static_cast<uint16_t>(refListStartForType[i] * refListEntrySize + resourceTypeListSize);

		if (!stream->WriteExact(&resTypeData, sizeof(resTypeData)))
			return ExportHouseResults::kIOError;
	}

	struct RefListEntry
	{
		BEInt16_t m_resID;
		BEInt16_t m_nameOffset;
		uint8_t m_attribs;
		uint8_t m_resDataStart[3];
		BEUInt32_t m_reserved;
	};

	// Write reference lists
	for (size_t ti = 0; ti < uniqueResTypes.Count(); ti++)
	{
		PortabilityLayer::ResTypeID resType = uniqueResTypes[ti];

		for (size_t i = 0; i < resources.Count(); i++)
		{
			const SimpleResource &res = resources[i];
			if (res.m_resType != resType)
				continue;

			RefListEntry refListEntry;
			refListEntry.m_resID = static_cast<int16_t>(res.m_resourceID);
			refListEntry.m_nameOffset = -1;
			refListEntry.m_attribs = res.m_attributes;

			const size_t resDataStart = res.m_offsetInResData;

			refListEntry.m_resDataStart[0] = static_cast<uint8_t>((resDataStart >> 16) & 0xff);
			refListEntry.m_resDataStart[1] = static_cast<uint8_t>((resDataStart >> 8) & 0xff);
			refListEntry.m_resDataStart[2] = static_cast<uint8_t>((resDataStart >> 0) & 0xff);
			refListEntry.m_reserved = 0;

			if (!stream->WriteExact(&refListEntry, sizeof(refListEntry)))
				return ExportHouseResults::kIOError;
		}
	}

	const GpUFilePos_t resForkEnd = stream->Tell();
	const GpUFilePos_t resMapSize = resForkEnd - resMapPos;

	struct ResForkHeader
	{
		BEUInt32_t m_resForkDataStart;
		BEUInt32_t m_resMapPos;
		BEUInt32_t m_resDataSize;
		BEUInt32_t m_resMapSize;
	};

	ResForkHeader header;
	header.m_resForkDataStart = static_cast<uint32_t>(resForkDataStart - resForkStart);
	header.m_resMapPos = static_cast<uint32_t>(resMapPos - resForkStart);
	header.m_resDataSize = static_cast<uint32_t>(resDataSize);
	header.m_resMapSize = static_cast<uint32_t>(resMapSize);

	// Write header at the start of the file
	if (!stream->SeekStart(resForkHeaderPos))
		return ExportHouseResults::kIOError;

	if (!stream->WriteExact(&header, sizeof(header)))
		return ExportHouseResults::kIOError;

	// Write header at the start of the resource map
	if (!stream->SeekStart(resMapPos))
		return ExportHouseResults::kIOError;

	if (!stream->WriteExact(&header, sizeof(header)))
		return ExportHouseResults::kIOError;

	// Return to the end of the file
	if (!stream->SeekStart(resForkEnd))
		return ExportHouseResults::kIOError;

	return ExportHouseResults::kOK;
}

ExportHouseResult_t TryExportHouseToStream(GpIOStream *stream)
{
	uint8_t mb2Header[PortabilityLayer::MacBinary2::kHeaderSize];
	memset(mb2Header, 0, sizeof(mb2Header));

	// Write MacBinary header
	if (!stream->WriteExact(mb2Header, sizeof(mb2Header)))
		return ExportHouseResults::kIOError;

	houseType *house = *thisHouse;
	const size_t houseSize = thisHouse.MMBlock()->m_size;
	const size_t nRooms = house->nRooms;
	const size_t houseDataSize = houseType::kBinaryDataSize + sizeof(roomType) * nRooms;
	ByteSwapHouse(house, houseSize, true);

	if (!stream->WriteExact(house, houseType::kBinaryDataSize))
		return ExportHouseResults::kIOError;

	if (!stream->WriteExact(house->rooms, sizeof(roomType) * nRooms))
		return ExportHouseResults::kIOError;

	ByteSwapHouse(house, houseSize, false);

	char padding[128];
	memset(padding, 0, sizeof(padding));

	const GpUFilePos_t dataAlignExcess = stream->Tell() % 128;
	if (dataAlignExcess != 0)
	{
		if (!stream->WriteExact(padding, 128 - dataAlignExcess))
			return ExportHouseResults::kIOError;
	}

	const GpUFilePos_t resForkPos = stream->Tell();

	// Serialize resources
	if (houseResFork != nullptr)
	{
		ExportHouseResult_t resExportResult = TryExportResources(stream, houseResFork);
		if (resExportResult != ExportHouseResults::kOK)
			return resExportResult;
	}

	const GpUFilePos_t resForkSize = stream->Tell() - resForkPos;

	const GpUFilePos_t resAlignExcess = resForkSize % 128;
	if (resForkSize != 0)
	{
		if (!stream->WriteExact(padding, 128 - resAlignExcess))
			return ExportHouseResults::kIOError;
	}

	PortabilityLayer::MacFileInfo fileInfo;
	fileInfo.m_fileName.Set(thisHouseName[0], reinterpret_cast<const char*>(thisHouseName + 1));
	fileInfo.m_commentSize = 0;
	fileInfo.m_dataForkSize = houseDataSize;
	fileInfo.m_resourceForkSize = resForkSize;
	memcpy(fileInfo.m_properties.m_fileType, "gliH", 4);
	memcpy(fileInfo.m_properties.m_fileCreator, "ozm5", 4);
	fileInfo.m_properties.m_xPos = 0;
	fileInfo.m_properties.m_yPos = 0;
	fileInfo.m_properties.m_finderFlags = 0;
	fileInfo.m_properties.m_protected = 0;
	fileInfo.m_properties.m_createdTimeMacEpoch = fileInfo.m_properties.m_modifiedTimeMacEpoch = PLDrivers::GetSystemServices()->GetTime();

	PortabilityLayer::MacBinary2::SerializeHeader(mb2Header, fileInfo);

	if (!stream->SeekStart(0))
		return ExportHouseResults::kIOError;

	if (!stream->WriteExact(mb2Header, PortabilityLayer::MacBinary2::kHeaderSize))
		return ExportHouseResults::kIOError;

	return ExportHouseResults::kOK;
}

ExportHouseResult_t TryExportHouse(void)
{
	GpIOStream *stream = nullptr;
	if (PortabilityLayer::FileManager::GetInstance()->OpenNonCompositeFile(PortabilityLayer::VirtualDirectories::kSourceExport, thisHouseName, ".bin", PortabilityLayer::EFilePermission_Write, GpFileCreationDispositions::kCreateOrOverwrite, stream))
		return ExportHouseResults::kStreamFailed;

	ExportHouseResult_t result = TryExportHouseToStream(stream);
	stream->Close();

	return result;
}

void ExportHouse(void)
{
	ExportHouseResult_t result = TryExportHouse();

	switch (result)
	{
	case ExportHouseResults::kOK:
		break;
	case ExportHouseResults::kMemError:
		YellowAlert(kYellowNoMemory, 0);
		break;
	case ExportHouseResults::kInternalError:
		YellowAlert(kYellowUnaccounted, 0);
		break;
	case ExportHouseResults::kIOError:
	case ExportHouseResults::kStreamFailed:
		YellowAlert(kYellowFailedWrite, 0);
		break;
	case ExportHouseResults::kResourceError:
		YellowAlert(kYellowFailedResOpen, 0);
		break;
	}
}
