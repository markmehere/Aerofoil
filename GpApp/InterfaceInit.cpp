
//============================================================================
//----------------------------------------------------------------------------
//								InterfaceInit.c
//----------------------------------------------------------------------------
//============================================================================


#include "Externs.h"
#include "Environ.h"
#include "HostDisplayDriver.h"
#include "IGpDisplayDriver.h"
#include "GpApplicationName.h"
#include "Map.h"
#include "MenuManager.h"
#include "PLKeyEncoding.h"
#include "PLPasStr.h"
#include "RectUtils.h"
#include "ResourceManager.h"
#include "Tools.h"


#define kHandCursorID		128
#define kVertCursorID		129
#define kHoriCursorID		130
#define kDiagCursorID		131

struct IGpCursor;

extern	THandle<Rect>	mirrorRects;
extern	WindowPtr		mapWindow, toolsWindow, linkWindow;
extern	Rect			boardSrcRect, localRoomsDest[];
extern	IGpCursor		*handCursor, *vertCursor, *horiCursor;
extern	IGpCursor		*diagCursor;
extern	MenuHandle		appleMenu, gameMenu, optionsMenu, houseMenu;
extern	long			incrementModeTime;
extern	UInt32			doubleTime;
extern	short			fadeInSequence[], idleMode;
extern	short			toolSelected, lastBackground, wasFlower, numExtraHouses;
extern	short			lastHighScore, maxFiles, willMaxFiles;
extern	Boolean			quitting, playing, fadeGraysOut;
extern	Boolean			houseOpen, newRoomNow, evenFrame, menusUp, demoGoing;
extern	Boolean			twoPlayerGame, paused, hasMirror, splashDrawn;


//==============================================================  Functions
//--------------------------------------------------------------  InitializeMenus

// The menus are loaded from disk and the menu bar set up and drawn.

void InitializeMenus (void)
{
	appleMenu = GetMenu(kAppleMenuID);
	if (appleMenu == nil)
		RedAlert(kErrFailedResourceLoad);
	//AppendResMenu(appleMenu, 'DRVR');	// GP: We don't support this
	AppendMenuItem(appleMenu, 0, 0, 0, 0, true, false, PSTR("About " GP_APPLICATION_NAME "\xc9"));
	AppendMenuItem(appleMenu, 0, 0, 0, 0, true, false, PSTR("Export Source Code\xc9"));
	InsertMenu(appleMenu, 0);
	
	gameMenu = GetMenu(kGameMenuID);
	if (gameMenu == nil)
		RedAlert(kErrFailedResourceLoad);
	InsertMenu(gameMenu, 0);
	
	optionsMenu = GetMenu(kOptionsMenuID);
	if (optionsMenu == nil)
		RedAlert(kErrFailedResourceLoad);
	InsertMenu(optionsMenu, 0);
	
	menusUp = true;
	PortabilityLayer::MenuManager::GetInstance()->SetMenuVisible(true);
	
	houseMenu = GetMenu(kHouseMenuID);
	if (houseMenu == nil)
		RedAlert(kErrFailedResourceLoad);
	
	UpdateMenus(false);
}

//--------------------------------------------------------------  GetExtraCursors

// Extra cursors (custom cursors) like the "hand" and various room…
// editing cursors are loaded up.

IGpCursor *LoadBWCursor(int resID)
{
	const THandle<void> resHdl = PortabilityLayer::ResourceManager::GetInstance()->GetAppResource('CURS', resID);
	if (!resHdl)
		return nullptr;

	struct BWCursor
	{
		uint8_t m_pixels[32];
		uint8_t m_mask[32];
		BEUInt16_t m_hotSpotX;
		BEUInt16_t m_hotSpotY;
	};

	const BWCursor *cursorData = static_cast<const BWCursor *>(*resHdl);

	IGpCursor *cursor = PortabilityLayer::HostDisplayDriver::GetInstance()->CreateBWCursor(16, 16, cursorData->m_pixels, cursorData->m_mask, cursorData->m_hotSpotX, cursorData->m_hotSpotY);
	resHdl.Dispose();

	return cursor;
}

void GetExtraCursors (void)
{
	struct BWCursor
	{
		uint8_t m_pixels[32];
		uint8_t m_mask[32];
		BEUInt16_t m_hotSpotX;
		BEUInt16_t m_hotSpotY;
	};

	handCursor = LoadBWCursor(kHandCursorID);
	if (handCursor == nil)
		RedAlert(kErrFailedResourceLoad);
	
	vertCursor = LoadBWCursor(kVertCursorID);
	if (vertCursor == nil)
		RedAlert(kErrFailedResourceLoad);
	
	horiCursor = LoadBWCursor(kHoriCursorID);
	if (horiCursor == nil)
		RedAlert(kErrFailedResourceLoad);
	
	diagCursor = LoadBWCursor(kDiagCursorID);
	if (diagCursor == nil)
		RedAlert(kErrFailedResourceLoad);
}

//--------------------------------------------------------------  RecomputeScreenRects
void RecomputeInterfaceRects (void)
{
	houseRect = thisMac.constrainedScreen;
	houseRect.bottom -= kScoreboardTall;
	if (houseRect.right > kMaxViewWidth)
		houseRect.right = kMaxViewWidth;
	if (houseRect.bottom > kMaxViewHeight)
		houseRect.bottom = kMaxViewHeight;

	// NOTE: This is actually buggy, since the visible area is houseRect, not screen.
	// We preserve the buggy behavior for authenticity unless the window is very tall.
	short poHeight = RectTall(&thisMac.constrainedScreen);
	if (poHeight > kMaxViewHeight - kScoreboardTall)
		poHeight = kMaxViewHeight - kScoreboardTall;

	playOriginH = (RectWide(&thisMac.constrainedScreen) - kRoomWide) / 2;
	playOriginV = (poHeight - kTileHigh) / 2;

	for (int i = 0; i < 9; i++)
	{
		QSetRect(&localRoomsDest[i], 0, 0, kRoomWide, kTileHigh);
		QOffsetRect(&localRoomsDest[i], playOriginH, playOriginV);
	}
	QOffsetRect(&localRoomsDest[kNorthRoom], 0, -kVertLocalOffset);
	QOffsetRect(&localRoomsDest[kNorthEastRoom], kRoomWide, -kVertLocalOffset);
	QOffsetRect(&localRoomsDest[kEastRoom], kRoomWide, 0);
	QOffsetRect(&localRoomsDest[kSouthEastRoom], kRoomWide, kVertLocalOffset);
	QOffsetRect(&localRoomsDest[kSouthRoom], 0, kVertLocalOffset);
	QOffsetRect(&localRoomsDest[kSouthWestRoom], -kRoomWide, kVertLocalOffset);
	QOffsetRect(&localRoomsDest[kWestRoom], -kRoomWide, 0);
	QOffsetRect(&localRoomsDest[kNorthWestRoom], -kRoomWide, -kVertLocalOffset);
}

//--------------------------------------------------------------  VariableInit

// All the simple interface variables are intialized here - Booleans,…
// shorts, a few Rects, etc.

void VariableInit (void)
{
	short		i;
	
	menusUp = false;
	quitting = false;
	houseOpen = false;
	newRoomNow = false;
	playing = false;
	evenFrame = false;
	if (thisMac.isDepth == 8)
		fadeGraysOut = true;
	else
		fadeGraysOut = false;
	twoPlayerGame = false;
	paused = false;
	hasMirror = false;
	demoGoing = false;
	scrapIsARoom = true;
	splashDrawn = false;
	
#ifndef COMPILEDEMO
//	SeeIfValidScrapAvailable(false);
#endif
	
	theGlider.which = kPlayer1;
	theGlider2.leftKey = PL_KEY_ASCII('A');
	theGlider2.rightKey = PL_KEY_ASCII('D');
	theGlider2.battKey = PL_KEY_ASCII('S');
	theGlider2.bandKey = PL_KEY_ASCII('W');
	theGlider2.gamepadLeftKey = PL_KEY_GAMEPAD_BUTTON(kDPadLeft, 1);
	theGlider2.gamepadRightKey = PL_KEY_GAMEPAD_BUTTON(kDPadRight, 1);
	theGlider2.gamepadBandKey = PL_KEY_GAMEPAD_BUTTON(kFaceDown, 1);
	theGlider2.gamepadBattKey = PL_KEY_GAMEPAD_BUTTON(kFaceLeft, 1);
	theGlider2.which = kPlayer2;
	
	theMode = kSplashMode;
	thisRoomNumber = 0;
	previousRoom = -1;
	toolSelected = kSelectTool;
	lastBackground = kBaseBackgroundID;
	wasFlower = RandomInt(kNumFlowers);
	lastHighScore = -1;
	idleMode = kIdleSplashMode;
	incrementModeTime = TickCount() + kIdleSplashTicks;
	willMaxFiles = maxFiles;
	numExtraHouses = 0;
	
	fadeInSequence[0] = 4;	// 4
	fadeInSequence[1] = 5;
	fadeInSequence[2] = 6;
	fadeInSequence[3] = 7;
	fadeInSequence[4] = 5;	// 5
	fadeInSequence[5] = 6;
	fadeInSequence[6] = 7;
	fadeInSequence[7] = 8;
	fadeInSequence[8] = 6;	// 6
	fadeInSequence[9] = 7;
	fadeInSequence[10] = 8;
	fadeInSequence[11] = 9;
	fadeInSequence[12] = 7;	// 7
	fadeInSequence[13] = 8;
	fadeInSequence[14] = 9;
	fadeInSequence[15] = 10;
	
	doubleTime = 30;	// PL_NotYetImplemented_TODO: Get this from the system settings
	
	mirrorRects = nil;
	mainWindow = nil;
	boardWindow = nil;
	mapWindow = nil;
	toolsWindow = nil;
	linkWindow = nil;
	coordWindow = nil;
	toolSrcMap = nil;
	nailSrcMap = nil;
	
	RecomputeInterfaceRects();
}
