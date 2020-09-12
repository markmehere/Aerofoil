//============================================================================
//----------------------------------------------------------------------------
//								Glider PRO 1.0.4
//								by  john calhoun
//----------------------------------------------------------------------------
//============================================================================


#include "PLApplication.h"
#include "PLKeyEncoding.h"
#include "Externs.h"
#include "Environ.h"
#include "HostDisplayDriver.h"
#include "IGpDisplayDriver.h"
#include "GpIOStream.h"
#include "House.h"
#include "WindowManager.h"


#define kPrefsVersion			0x0038


void ReadInPrefs (void);
void WriteOutPrefs (void);
int main(int argc, const char **argv);


short		isVolume, wasVolume;
short		isDepthPref, dataResFile, numSMWarnings;
Boolean		quitting, doZooms, quickerTransitions, isAutoScale, isUseICCProfile;


extern Str31		highBanner;
extern Str15		leftName, rightName, batteryName, bandName;
extern Str15		highName;
//extern long		encryptedNumber;
extern short		maxFiles, numNeighbors, willMaxFiles;
extern GpIOStream	*houseStream;
extern short		isEditH, isEditV, isMapH, isMapV;
extern short		isToolsH, isToolsV, isCoordH, isCoordV;
extern short		isLinkH, isLinkV, toolMode, mapLeftRoom, mapTopRoom;
extern short		mapRoomsWide, mapRoomsHigh, wasFloor, wasSuite;
extern Boolean		isMusicOn, isSoundOn, isPlayMusicIdle, isHouseChecks;
extern Boolean		houseOpen, isDoColorFade, isEscPauseKey;
extern Boolean		autoRoomEdit, doAutoDemo, doBackground;
extern Boolean		isMapOpen, isToolsOpen, isCoordOpen;
extern Boolean		doPrettyMap, doBitchDialogs;
//extern Boolean		didValidation;

THandle<void>		globalModulePrefs;

//==============================================================  Functions
//--------------------------------------------------------------  ReadInPrefs

// Called only once when game launches - reads in the preferences saved…
// from the last time Glider PRO was launched.  If no prefs are found,…
// it assigns default settings.

void ReadInPrefs (void)
{
	prefsInfo	thePrefs;

	THandle<void> modulePrefs;
	
	if (LoadPrefs(&thePrefs, &modulePrefs, kPrefsVersion))
	{
#ifdef COMPILEDEMO
		PasStringCopy("\pDemo House", thisHouseName);
#else
		PasStringCopy(thePrefs.wasDefaultName, thisHouseName);
#endif
		PasStringCopy(thePrefs.wasLeftName, leftName);
		PasStringCopy(thePrefs.wasRightName, rightName);
		PasStringCopy(thePrefs.wasBattName, batteryName);
		PasStringCopy(thePrefs.wasBandName, bandName);
		PasStringCopy(thePrefs.wasHighName, highName);
		PasStringCopy(thePrefs.wasHighBanner, highBanner);
		theGlider.leftKey = thePrefs.wasLeftMap;
		theGlider.rightKey = thePrefs.wasRightMap;
		theGlider.battKey = thePrefs.wasBattMap;
		theGlider.bandKey = thePrefs.wasBandMap;
		theGlider.gamepadLeftKey = thePrefs.wasGPLeftMap;
		theGlider.gamepadRightKey = thePrefs.wasGPRightMap;
		theGlider.gamepadBandKey = thePrefs.wasGPBandMap;
		theGlider.gamepadBattKey = thePrefs.wasGPBattMap;
		theGlider.gamepadFlipKey = thePrefs.wasGPFlipMap;
		theGlider.gamepadFaceRightKey = thePrefs.wasGPFaceRightMap;
		theGlider.gamepadFaceLeftKey = thePrefs.wasGPFaceLeftMap;
#ifndef COMPILEDEMO
#ifndef COMPILENOCP
		encryptedNumber = thePrefs.encrypted;
#endif			// COMPILENOCP
#endif			// COMPILEDEMO
		isVolume = thePrefs.wasVolume;
		isDepthPref = thePrefs.wasDepthPref;
		isMusicOn = thePrefs.wasMusicOn;
		doZooms = thePrefs.wasZooms;
		quickerTransitions = thePrefs.wasQuickTrans;
		isDoColorFade = thePrefs.wasDoColorFade;
		isPlayMusicIdle = thePrefs.wasIdleMusic;
		isPlayMusicGame = thePrefs.wasGameMusic;
		isHouseChecks = thePrefs.wasHouseChecks;
		maxFiles = thePrefs.wasMaxFiles;
		if ((maxFiles < 12) || (maxFiles > 500))
			maxFiles = 12;
		isEditH = thePrefs.wasEditH;
		isEditV = thePrefs.wasEditV;
		isMapH = thePrefs.wasMapH;
		isMapV = thePrefs.wasMapV;
		mapRoomsWide = thePrefs.wasMapWide;
		mapRoomsHigh = thePrefs.wasMapHigh;
		isToolsH = thePrefs.wasToolsH;
		isToolsV = thePrefs.wasToolsV;
		isLinkH = thePrefs.wasLinkH;
		isLinkV = thePrefs.wasLinkV;
		isCoordH = thePrefs.wasCoordH;
		isCoordV = thePrefs.wasCoordV;
		mapLeftRoom = thePrefs.isMapLeft;
		mapTopRoom = thePrefs.isMapTop;
		wasFloor = thePrefs.wasFloor;
		wasSuite = thePrefs.wasSuite;
		numSMWarnings = thePrefs.smWarnings;
		autoRoomEdit = thePrefs.wasAutoEdit;
		isMapOpen = thePrefs.wasMapOpen;
		isToolsOpen = thePrefs.wasToolsOpen;
		isCoordOpen = thePrefs.wasCoordOpen;
		numNeighbors = thePrefs.wasNumNeighbors;
		toolMode = thePrefs.wasToolGroup;
		doAutoDemo = thePrefs.wasDoAutoDemo;
		isEscPauseKey = thePrefs.wasEscPauseKey;
		isAutoScale = thePrefs.wasAutoScale;
		isUseICCProfile = thePrefs.wasUseICCProfile;
		doBackground = thePrefs.wasDoBackground;
		doPrettyMap = thePrefs.wasPrettyMap;
		doBitchDialogs = thePrefs.wasBitchDialogs;

		if (modulePrefs)
			ApplyModulePrefs(&modulePrefs);

		globalModulePrefs.Dispose();
		globalModulePrefs = modulePrefs;
		modulePrefs = nullptr;
	}
	else
	{
#ifdef COMPILEDEMO
		PasStringCopy("\pDemo House", thisHouseName);
#else
		PasStringCopy(PSTR("Slumberland"), thisHouseName);
#endif
		PasStringCopy(PSTR("lf arrow"), leftName);
		PasStringCopy(PSTR("rt arrow"), rightName);
		PasStringCopy(PSTR("dn arrow"), batteryName);
		PasStringCopy(PSTR("up arrow"), bandName);
		PasStringCopy(PSTR("Your Name"), highName);
		PasStringCopy(PSTR("Your Message Here"), highBanner);
		theGlider.leftKey = PL_KEY_SPECIAL(kLeftArrow);
		theGlider.rightKey = PL_KEY_SPECIAL(kRightArrow);
		theGlider.battKey = PL_KEY_SPECIAL(kDownArrow);
		theGlider.bandKey = PL_KEY_SPECIAL(kUpArrow);
		theGlider.gamepadLeftKey = PL_KEY_GAMEPAD_BUTTON(kDPadLeft, 0);
		theGlider.gamepadRightKey = PL_KEY_GAMEPAD_BUTTON(kDPadRight, 0);
		theGlider.gamepadBandKey = PL_KEY_GAMEPAD_BUTTON(kFaceDown, 0);
		theGlider.gamepadBattKey = PL_KEY_GAMEPAD_BUTTON(kFaceLeft, 0);
		theGlider.gamepadFlipKey = PL_KEY_GAMEPAD_BUTTON(kFaceUp, 0);
		theGlider.gamepadFaceRightKey = PL_KEY_GAMEPAD_BUTTON(kRightBumper, 0);
		theGlider.gamepadFaceLeftKey = PL_KEY_GAMEPAD_BUTTON(kLeftBumper, 0);

		UnivGetSoundVolume(&isVolume, thisMac.hasSM3);
		if (isVolume < 1)
			isVolume = 1;
		else if (isVolume > 3)
			isVolume = 3;
		
		isDepthPref = kSwitchIfNeeded;
		isSoundOn = true;
		isMusicOn = true;
		isPlayMusicIdle = true;
		isPlayMusicGame = true;
		isHouseChecks = true;
		doZooms = true;
		quickerTransitions = false;
		numNeighbors = 9;
		isDoColorFade = true;
		maxFiles = 48;
		willMaxFiles = 48;
		isEditH = 3;
		isEditV = 41;
		isMapH = 3;
//		isMapV = qd.screenBits.bounds.bottom - 100;
		isMapV = 100;
		mapRoomsWide = 15;
		mapRoomsHigh = 4;
//		isToolsH = qd.screenBits.bounds.right - 120;
		isToolsH = 100;
		isToolsV = 35;
		isLinkH = 50;
		isLinkV = 80;
//		isCoordH = qd.screenBits.bounds.right - 55;
		isCoordH = 50;
		isCoordV = 204;
		mapLeftRoom = 60;
		mapTopRoom = 50;
		wasFloor = 0;
		wasSuite = 0;
		numSMWarnings = 0;
		autoRoomEdit = true;
		isMapOpen = true;
		isToolsOpen = true;
		isCoordOpen = false;
		toolMode = kBlowerMode;
		doAutoDemo = true;
		isEscPauseKey = false;
		isAutoScale = true;
		isUseICCProfile = true;
		doBackground = false;
		doPrettyMap = false;
		doBitchDialogs = true;

		IGpDisplayDriver *displayDriver = PortabilityLayer::HostDisplayDriver::GetInstance();
		if (!displayDriver->IsFullScreen())
			displayDriver->RequestToggleFullScreen(0);

		modulePrefs.Dispose();
	}
	
	if ((numNeighbors > 1) && (thisMac.constrainedScreen.right <= 512))
		numNeighbors = 1;
	
	UnivGetSoundVolume(&wasVolume, thisMac.hasSM3);
	UnivSetSoundVolume(isVolume, thisMac.hasSM3);
	
	if (isVolume == 0)
		isSoundOn = false;
	else
		isSoundOn = true;
}

//--------------------------------------------------------------  WriteOutPrefs

// Called just before Glider PRO quits.  This function writes out…
// the user preferences to disk.

void WriteOutPrefs (void)
{
	prefsInfo	thePrefs;	
	
	UnivGetSoundVolume(&isVolume, thisMac.hasSM3);
	
#ifdef COMPILEDEMO
	PasStringCopy("\pDemo House", thePrefs.wasDefaultName);
#else
	PasStringCopy(thisHouseName, thePrefs.wasDefaultName);
#endif
	PasStringCopy(leftName, thePrefs.wasLeftName);
	PasStringCopy(rightName, thePrefs.wasRightName);
	PasStringCopy(batteryName, thePrefs.wasBattName);
	PasStringCopy(bandName, thePrefs.wasBandName);
	PasStringCopy(highName, thePrefs.wasHighName);
	PasStringCopy(highBanner, thePrefs.wasHighBanner);
	thePrefs.wasLeftMap = theGlider.leftKey;
	thePrefs.wasRightMap = theGlider.rightKey;
	thePrefs.wasBattMap = theGlider.battKey;
	thePrefs.wasBandMap = theGlider.bandKey;
	thePrefs.wasGPLeftMap = theGlider.gamepadLeftKey;
	thePrefs.wasGPRightMap = theGlider.gamepadRightKey;
	thePrefs.wasGPBattMap = theGlider.gamepadBattKey;
	thePrefs.wasGPBandMap = theGlider.gamepadBandKey;
	thePrefs.wasGPFlipMap = theGlider.gamepadFlipKey;
	thePrefs.wasGPFaceLeftMap = theGlider.gamepadFaceLeftKey;
	thePrefs.wasGPFaceRightMap = theGlider.gamepadFaceRightKey;
#ifndef COMPILEDEMO
#ifndef COMPILENOCP
	thePrefs.encrypted = encryptedNumber;
	thePrefs.fakeLong = Random();
#endif			// COMPILENOCP
#endif			// COMPILEDEMO
	thePrefs.wasVolume = isVolume;
	thePrefs.wasDepthPref = isDepthPref;
	thePrefs.wasMusicOn = isMusicOn;
	thePrefs.wasZooms = doZooms;
	thePrefs.wasQuickTrans = quickerTransitions;
	thePrefs.wasDoColorFade = isDoColorFade;
	thePrefs.wasIdleMusic = isPlayMusicIdle;
	thePrefs.wasGameMusic = isPlayMusicGame;
	thePrefs.wasHouseChecks = isHouseChecks;
	thePrefs.wasMaxFiles = willMaxFiles;
	thePrefs.wasEditH = isEditH;
	thePrefs.wasEditV = isEditV;
	thePrefs.wasMapH = isMapH;
	thePrefs.wasMapV = isMapV;
	thePrefs.wasMapWide = mapRoomsWide;
	thePrefs.wasMapHigh = mapRoomsHigh;
	thePrefs.wasToolsH = isToolsH;
	thePrefs.wasToolsV = isToolsV;
	thePrefs.isMapLeft = mapLeftRoom;
	thePrefs.isMapTop = mapTopRoom;
	thePrefs.wasFloor = wasFloor;
	thePrefs.wasSuite = wasSuite;
	thePrefs.wasLinkH = isLinkH;
	thePrefs.wasLinkV = isLinkV;
	thePrefs.wasCoordH = isCoordH;
	thePrefs.wasCoordV = isCoordV;
	thePrefs.smWarnings = numSMWarnings;
	thePrefs.wasAutoEdit = autoRoomEdit;
	thePrefs.wasMapOpen = isMapOpen;
	thePrefs.wasToolsOpen = isToolsOpen;
	thePrefs.wasCoordOpen = isCoordOpen;
	thePrefs.wasNumNeighbors = numNeighbors;
	thePrefs.wasToolGroup = toolMode;
	thePrefs.wasDoAutoDemo = doAutoDemo;
	thePrefs.wasEscPauseKey = isEscPauseKey;
	thePrefs.wasAutoScale = isAutoScale;
	thePrefs.wasUseICCProfile = isUseICCProfile;
	thePrefs.wasDoBackground = doBackground;
	thePrefs.wasPrettyMap = doPrettyMap;
	thePrefs.wasBitchDialogs = doBitchDialogs;

	THandle<void> modulePrefs;

	if (!SaveModulePrefs(globalModulePrefs, &modulePrefs) || !SavePrefs(&thePrefs, &modulePrefs, kPrefsVersion))
		SysBeep(1);

	modulePrefs.Dispose();
	
	UnivSetSoundVolume(wasVolume, thisMac.hasSM3);
}

//--------------------------------------------------------------  main
// Here is main().  The first function called when Glider PRO comes up.

int gpAppMain()
{
//	long		wasSeed;
	long		theErr;
	PLError_t		fileErr;
	Boolean		whoCares, copyGood;

	PL_Init();
	
	ToolBoxInit();
	CheckOurEnvirons();
	InstallResolutionHandler();
	if (!thisMac.hasColor)
		RedAlert(kErrNeedColorQD);
	if (!thisMac.hasSystem7)
		RedAlert(kErrNeedSystem7);
	if (thisMac.numScreens == 0)
		RedAlert(kErrNeed16Or256Colors);
//	dataResFile = OpenResFile("\pMermaid");
	SetUpAppleEvents();
	LoadCursors();
	ReadInPrefs();

	SpinCursor(2);	// Tick once to let the display driver flush any resolution changes from prefs
	FlushResolutionChange();
	
#if defined COMPILEDEMO
	copyGood = true;
#elif defined COMPILENOCP
//	didValidation = false;
	copyGood = true;
#else
	didValidation = false;
	copyGood = ValidInstallation(true);
	if (!copyGood)
		encryptedNumber = 0L;
	else if (didValidation)
		WriteOutPrefs();				SpinCursor(3);
#endif

//	if ((thisMac.numScreens > 1) && (isUseSecondScreen))
//		ReflectSecondMonitorEnvirons(false, true, true);
	HandleDepthSwitching();
	VariableInit();						SpinCursor(2);
	CheckMemorySize();
	GetExtraCursors();					SpinCursor(2);
	InitMarquee();
	CreatePointers();					SpinCursor(2);
	InitSrcRects();
	CreateOffscreens();					SpinCursor(2);
	OpenMainWindow();

	if (isDoColorFade)
		PortabilityLayer::WindowManager::GetInstance()->SetWindowDesaturation(mainWindow, 1.0);

	InitSound();						SpinCursor(2);
	InitMusic();						SpinCursor(2);
	BuildHouseList();
	if (OpenHouse())
		whoCares = ReadHouse();
	
	PlayPrioritySound(kBirdSound, kBirdPriority);
	DelayTicks(6);
	InitializeMenus();					InitCursor();
	
#if BUILD_ARCADE_VERSION
//	HideMenuBarOld();
#endif
	
	if (isDoColorFade)
		WashColorIn();

	UpdateMainWindow();
	
	while (!quitting)		// this is the main loop
		HandleEvent();
/*	
#if BUILD_ARCADE_VERSION
	ShowMenuBarOld();
#endif
*/	
	KillMusic();
	KillSound();
	if (houseOpen)
	{
		if (!CloseHouse())
		{
			CloseHouseResFork();
			houseStream->Close();
			houseOpen = false;
		}
	}
	WriteOutPrefs();
	PL_DEAD(FlushEvents());
//	theErr = LoadScrap();

	return 0;
}

