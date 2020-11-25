//============================================================================
//----------------------------------------------------------------------------
//								Glider PRO 1.0.4
//								by  john calhoun
//----------------------------------------------------------------------------
//============================================================================


#include "WindowDef.h"
#include "BitmapImage.h"
#include "Externs.h"
#include "Environ.h"
#include "FontFamily.h"
#include "GpRenderedFontMetrics.h"
#include "IGpMutex.h"
#include "IGpThreadEvent.h"
#include "IGpDisplayDriver.h"
#include "IGpSystemServices.h"
#include "GpIOStream.h"
#include "House.h"
#include "MainMenuUI.h"
#include "MemoryManager.h"
#include "MenuManager.h"
#include "QDPixMap.h"
#include "QDStandardPalette.h"
#include "RenderedFont.h"
#include "ResolveCachingColor.h"
#include "ResourceManager.h"
#include "Utilities.h"
#include "WindowManager.h"
#include "WorkerThread.h"

#include "PLApplication.h"
#include "PLDrivers.h"
#include "PLKeyEncoding.h"
#include "PLStandardColors.h"
#include "PLSysCalls.h"

#include <atomic>

int loadScreenWindowPhase;
int loadScreenRingStep;
WindowPtr loadScreenWindow;
Rect loadScreenProgressBarRect;
int loadScreenProgress;
DrawSurface *loadScreenRingSurface;


#define kPrefsVersion			0x0038


void ReadInPrefs (void);
void WriteOutPrefs (void);
int main(int argc, const char **argv);


short		isVolume, wasVolume;
short		isDepthPref, dataResFile, numSMWarnings;
Boolean		quitting, doZooms, quickerTransitions, isUseICCProfile, isPrefsLoaded;
Boolean		isAutoScale = true;


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
extern Boolean		doPrettyMap, doComplainDialogs;
//extern Boolean		didValidation;

THandle<void>		globalModulePrefs;

//==============================================================  Functions
//--------------------------------------------------------------  ReadInPrefs

// Called only once when game launches - reads in the preferences saved?
// from the last time Glider PRO was launched.  If no prefs are found,?
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
		doComplainDialogs = thePrefs.wasComplainDialogs;

		if (modulePrefs)
			ApplyModulePrefs(&modulePrefs);

		globalModulePrefs.Dispose();
		globalModulePrefs = modulePrefs;
		modulePrefs = nullptr;

		isPrefsLoaded = true;
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
		doComplainDialogs = true;

		IGpDisplayDriver *displayDriver = PLDrivers::GetDisplayDriver();
		if (PLDrivers::GetSystemServices()->IsFullscreenPreferred() != displayDriver->IsFullScreen())
			displayDriver->RequestToggleFullScreen(0);

		modulePrefs.Dispose();

		isPrefsLoaded = false;
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

// Called just before Glider PRO quits.  This function writes out?
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
	thePrefs.wasComplainDialogs = doComplainDialogs;

	THandle<void> modulePrefs;

	if (!SaveModulePrefs(globalModulePrefs, &modulePrefs) || !SavePrefs(&thePrefs, &modulePrefs, kPrefsVersion))
		SysBeep(1);

	modulePrefs.Dispose();

	UnivSetSoundVolume(wasVolume, thisMac.hasSM3);
}

void StepLoadScreenRing()
{
	if (loadScreenWindow)
	{
		const int loadScreenStepGranularity = 2;
		loadScreenRingStep++;
		if (loadScreenRingStep == 24 * loadScreenStepGranularity)
			loadScreenRingStep = 0;

		Rect ringDestRect = Rect::Create(8, 8, 24, 24);
		Rect ringSrcRect = Rect::Create(0, 0, 16, 16) + Point::Create((loadScreenRingStep / loadScreenStepGranularity) * 16, 0);

		if (loadScreenWindowPhase == 0)
		{
			ringDestRect = Rect::Create(0, 0, 64, 64);

			const int progression = (loadScreenRingStep / loadScreenStepGranularity);
			ringSrcRect = Rect::Create(0, 0, 64, 64) + Point::Create((progression % 6) * 64, (progression / 6) * 64);
		}

		CopyBits(*loadScreenRingSurface->m_port.GetPixMap(), *loadScreenWindow->GetDrawSurface()->m_port.GetPixMap(), &ringSrcRect, &ringDestRect, srcCopy);
		loadScreenWindow->GetDrawSurface()->m_port.SetDirty(PortabilityLayer::QDPortDirtyFlag_Contents);
	}
}


void CreateLoadScreenWindow(int phase)
{
	if (loadScreenRingSurface)
	{
		DisposeGWorld(loadScreenRingSurface);
		loadScreenRingSurface = nullptr;
	}

	if (loadScreenWindow)
	{
		PortabilityLayer::WindowManager::GetInstance()->DestroyWindow(loadScreenWindow);
		loadScreenWindow = nullptr;
	}

	if (phase == 0)
	{
		static const int kLoadScreenHeight = 64;
		static const int kLoadScreenWidth = 64;
		static const int kLoadRingResource = 1303;

		ForceSyncFrame();
		PLSysCalls::Sleep(1);

		THandle<BitmapImage> loadRingImageH = PortabilityLayer::ResourceManager::GetInstance()->GetAppResource('PICT', kLoadRingResource).StaticCast<BitmapImage>();
		BitmapImage *loadRingImage = *loadRingImageH;

		DrawSurface *loadRingSurface = nullptr;
		Rect loadRingRect = loadRingImage->GetRect();
		loadRingRect.right *= 2;
		loadRingRect.bottom *= 2;
		CreateOffScreenGWorld(&loadRingSurface, &loadRingRect);
		loadRingSurface->DrawPicture(loadRingImageH, loadRingRect);

		int32_t lsX = (thisMac.fullScreen.Width() - kLoadScreenWidth) / 2;
		int32_t lsY = (thisMac.fullScreen.Height() - kLoadScreenHeight) / 2;


		const Rect loadScreenRect = Rect::Create(lsY, lsX, lsY + kLoadScreenHeight, lsX + kLoadScreenWidth);
		const Rect loadScreenLocalRect = Rect::Create(0, 0, loadScreenRect.Height(), loadScreenRect.Width());

		PortabilityLayer::WindowDef def = PortabilityLayer::WindowDef::Create(loadScreenRect, PortabilityLayer::WindowStyleFlags::kBorderless, true, 0, 0, PSTR(""));

		loadScreenWindow = PortabilityLayer::WindowManager::GetInstance()->CreateWindow(def);
		PortabilityLayer::WindowManager::GetInstance()->PutWindowBehind(loadScreenWindow, PL_GetPutInFrontWindowPtr());

		DrawSurface *surface = loadScreenWindow->GetDrawSurface();
		PortabilityLayer::ResolveCachingColor blackColor(StdColors::Black());

		surface->FillRect(loadScreenLocalRect, blackColor);

		loadScreenProgressBarRect = Rect::Create(0, 0, 0, 0);
		loadScreenProgress = 0;

		Rect ringDestRect = Rect::Create(0, 0, 64, 64);
		Rect ringSrcRect = Rect::Create(0, 0, 64, 64);
		CopyBits(*loadRingSurface->m_port.GetPixMap(), *surface->m_port.GetPixMap(), &ringSrcRect, &ringDestRect, srcCopy);

		loadRingImageH.Dispose();

		loadScreenRingSurface = loadRingSurface;
	}
	else if (phase == 1)
	{
		static const int kLoadScreenHeight = 32;
		static const int kLoadRingResource = 1302;

		int kLoadScreenWidth = 296;
		PLPasStr loadingText = PSTR("Loading...");

		if (!isPrefsLoaded)
		{
			loadingText = PSTR("Getting some things ready...");
			kLoadScreenWidth = 440;
		}

		ForceSyncFrame();
		PLSysCalls::Sleep(1);

		THandle<BitmapImage> loadRingImageH = PortabilityLayer::ResourceManager::GetInstance()->GetAppResource('PICT', kLoadRingResource).StaticCast<BitmapImage>();
		BitmapImage *loadRingImage = *loadRingImageH;

		DrawSurface *loadRingSurface = nullptr;
		Rect loadRingRect = loadRingImage->GetRect();
		CreateOffScreenGWorld(&loadRingSurface, &loadRingRect);
		loadRingSurface->DrawPicture(loadRingImageH, loadRingRect);

		int32_t lsX = (thisMac.fullScreen.Width() - kLoadScreenWidth) / 2;
		int32_t lsY = (thisMac.fullScreen.Height() - kLoadScreenHeight) / 2;


		const Rect loadScreenRect = Rect::Create(lsY, lsX, lsY + kLoadScreenHeight, lsX + kLoadScreenWidth);
		const Rect loadScreenLocalRect = Rect::Create(0, 0, loadScreenRect.Height(), loadScreenRect.Width());

		PortabilityLayer::WindowDef def = PortabilityLayer::WindowDef::Create(loadScreenRect, PortabilityLayer::WindowStyleFlags::kAlert, true, 0, 0, PSTR(""));

		loadScreenWindow = PortabilityLayer::WindowManager::GetInstance()->CreateWindow(def);
		PortabilityLayer::WindowManager::GetInstance()->PutWindowBehind(loadScreenWindow, PL_GetPutInFrontWindowPtr());

		DrawSurface *surface = loadScreenWindow->GetDrawSurface();
		PortabilityLayer::ResolveCachingColor blackColor(StdColors::Black());
		PortabilityLayer::ResolveCachingColor whiteColor(StdColors::White());

		surface->FillRect(loadScreenLocalRect, whiteColor);

		PortabilityLayer::WindowManager::GetInstance()->FlickerWindowIn(loadScreenWindow, 32);

		PortabilityLayer::RenderedFont *font = GetApplicationFont(18, PortabilityLayer::FontFamilyFlag_None, true);
		int32_t textY = (kLoadScreenHeight + font->GetMetrics().m_ascent) / 2;
		surface->DrawString(Point::Create(4 + 16 + 8, textY), loadingText, blackColor, font);

		static const int32_t loadBarPadding = 16;
		static const int32_t loadBarHeight = 10;
		int32_t loadBarStartX = static_cast<int32_t>(font->MeasureString(loadingText.UChars(), loadingText.Length())) + 4 + 16 + 8 + loadBarPadding;
		int32_t loadBarEndX = loadScreenLocalRect.right - loadBarPadding;

		loadScreenProgressBarRect = Rect::Create((loadScreenLocalRect.Height() - loadBarHeight) / 2, loadBarStartX, (loadScreenLocalRect.Height() + loadBarHeight) / 2, loadBarEndX);

		PortabilityLayer::ResolveCachingColor partialFillColor(PortabilityLayer::RGBAColor::Create(255, 255, 204, 255));
		surface->FrameRect(loadScreenProgressBarRect, blackColor);
		surface->FillRect(loadScreenProgressBarRect.Inset(1, 1), partialFillColor);

		Rect ringDestRect = Rect::Create(8, 8, 24, 24);
		Rect ringSrcRect = Rect::Create(0, 0, 16, 16);
		CopyBits(*loadRingSurface->m_port.GetPixMap(), *surface->m_port.GetPixMap(), &ringSrcRect, &ringDestRect, srcCopy);

		loadRingImageH.Dispose();

		loadScreenRingSurface = loadRingSurface;
	}
}

void StepLoadScreen(int steps, bool insertDelay)
{
	if (loadScreenWindow)
	{
		static const int kLoadScreenPhaseSwitchBreakpoint = 20;

		int oldProgress = loadScreenProgress;

		if (loadScreenProgress + steps >= kLoadScreenPhaseSwitchBreakpoint && loadScreenWindowPhase == 0)
		{
			loadScreenWindowPhase = 1;
			CreateLoadScreenWindow(loadScreenWindowPhase);

			// Reset old progress since the progress bar was redrawn
			oldProgress = 0;
		}

		const Rect loadScreenProgressBarFillRect = loadScreenProgressBarRect.Inset(1, 1);

		int loadScreenMax = 42;
		loadScreenProgress = loadScreenProgress + steps;
		if (loadScreenProgress > loadScreenMax)
			loadScreenProgress = loadScreenMax;

		PortabilityLayer::ResolveCachingColor fillColor(PortabilityLayer::RGBAColor::Create(51, 51, 51, 255));

		int prevStep = oldProgress * loadScreenProgressBarFillRect.Width() / loadScreenMax;
		int thisStep = loadScreenProgress * loadScreenProgressBarFillRect.Width() / loadScreenMax;

		loadScreenWindow->GetDrawSurface()->FillRect(Rect::Create(loadScreenProgressBarFillRect.top, loadScreenProgressBarFillRect.left + prevStep, loadScreenProgressBarFillRect.bottom, loadScreenProgressBarFillRect.left + thisStep), fillColor);
		ForceSyncFrame();

		if (insertDelay)
		{
			StepLoadScreenRing();
			Delay(1, nullptr);
		}
	}
	else
		SpinCursor(steps);
}

void InitLoadingWindow()
{
	// Only phones are slow enough for this to matter
	if (!thisMac.isTouchscreen)
		return;

	if (isPrefsLoaded)
		loadScreenWindowPhase = 1;
	else
		loadScreenWindowPhase = 0;

	CreateLoadScreenWindow(loadScreenWindowPhase);
}

enum PreloadFontCategory
{
	FontCategory_System,
	FontCategory_Application,
	FontCategory_Handwriting,
	FontCategory_Monospace,
};

struct PreloadFontSpec
{
	PreloadFontCategory m_category;
	int m_size;
	int m_flags;
	bool m_aa;
};

struct PreloadFontWorkSlot
{
	IGpThreadEvent *m_completedEvent;
	PortabilityLayer::WorkerThread *m_workerThread;
	std::atomic<int> m_singleJobCompleted;
	const PreloadFontSpec *m_spec;
	bool m_queued;

	PreloadFontWorkSlot();
	~PreloadFontWorkSlot();
};

PreloadFontWorkSlot::PreloadFontWorkSlot()
	: m_completedEvent(nullptr)
	, m_workerThread(nullptr)
	, m_spec(nullptr)
	, m_queued(false)
{
}

PreloadFontWorkSlot::~PreloadFontWorkSlot()
{
	if (m_workerThread)
		m_workerThread->Destroy();
}

void PreloadSingleFont (const PreloadFontSpec &spec)
{
	switch (spec.m_category)
	{
	case FontCategory_Application:
		GetApplicationFont(spec.m_size, spec.m_flags, spec.m_aa);
		break;
	case FontCategory_System:
		GetSystemFont(spec.m_size, spec.m_flags, spec.m_aa);
		break;
	case FontCategory_Handwriting:
		GetHandwritingFont(spec.m_size, spec.m_flags, spec.m_aa);
		break;
	case FontCategory_Monospace:
		GetMonospaceFont(spec.m_size, spec.m_flags, spec.m_aa);
		break;
	default:
		break;
	}
}

void PreloadFontThreadFunc(void *context)
{
	PreloadFontWorkSlot *wSlot = static_cast<PreloadFontWorkSlot*>(context);

	PreloadSingleFont(*wSlot->m_spec);
	wSlot->m_singleJobCompleted.fetch_add(1, std::memory_order_release);
	wSlot->m_completedEvent->Signal();
}

void PreloadFonts()
{
	static PreloadFontSpec specs[] =
	{
		// First entry should be the one needed to show the load screen
		{ FontCategory_Application, 18, PortabilityLayer::FontFamilyFlag_None, true },

		{ FontCategory_System, 9, PortabilityLayer::FontFamilyFlag_Bold, true },
		{ FontCategory_System, 10, PortabilityLayer::FontFamilyFlag_Bold, true },
		{ FontCategory_System, 12, PortabilityLayer::FontFamilyFlag_None, true },
		{ FontCategory_System, 12, PortabilityLayer::FontFamilyFlag_Bold, true },
		{ FontCategory_Application, 8, PortabilityLayer::FontFamilyFlag_None, true },
		{ FontCategory_Application, 9, PortabilityLayer::FontFamilyFlag_None, true },
		{ FontCategory_Application, 12, PortabilityLayer::FontFamilyFlag_Bold, true },
		{ FontCategory_Application, 14, PortabilityLayer::FontFamilyFlag_Bold, true },
		{ FontCategory_Application, 40, PortabilityLayer::FontFamilyFlag_None, true },
		{ FontCategory_Handwriting, 24, PortabilityLayer::FontFamilyFlag_None, true },
		{ FontCategory_Handwriting, 48, PortabilityLayer::FontFamilyFlag_None, true },
		{ FontCategory_Monospace, 10, PortabilityLayer::FontFamilyFlag_None, true },
	};

	PortabilityLayer::MemoryManager *mm = PortabilityLayer::MemoryManager::GetInstance();


	const int numFontSpecs = sizeof(specs) / sizeof(specs[0]);

	int queuedSpecs = 0;
	int completedSpecs = 0;

	// We can't actually slot these because FT isn't thread-safe when accessing the same font,
	// but we can do this to unclog the render thread.
	PreloadFontWorkSlot slot;
	slot.m_workerThread = PortabilityLayer::WorkerThread::Create();
	slot.m_completedEvent = PLDrivers::GetSystemServices()->CreateThreadEvent(true, false);

	while (completedSpecs < numFontSpecs)
	{
		if (slot.m_queued)
		{
			if (slot.m_singleJobCompleted.load(std::memory_order_acquire) != 0)
			{
				slot.m_completedEvent->Wait();
				slot.m_queued = false;
				completedSpecs++;

				StepLoadScreen(1, false);
			}
		}

		if (!slot.m_queued)
		{
			if (queuedSpecs < numFontSpecs)
			{
				slot.m_queued = true;
				slot.m_spec = specs + queuedSpecs;
				slot.m_singleJobCompleted.store(0, std::memory_order_release);
				slot.m_workerThread->AsyncExecuteTask(PreloadFontThreadFunc, &slot);

				queuedSpecs++;
			}
		}

		StepLoadScreenRing();
		Delay(1, nullptr);
	}
}

struct PreloadAATableSpec
{
	PortabilityLayer::RGBAColor m_color;
	bool m_isTone;
};

struct PreloadAATableWorkSlot
{
	IGpThreadEvent *m_completedEvent;
	IGpMutex *m_mutex;
	PortabilityLayer::WorkerThread *m_workerThread;
	std::atomic<int> m_singleJobCompleted;
	const PreloadAATableSpec *m_spec;
	bool m_queued;

	PreloadAATableWorkSlot();
	~PreloadAATableWorkSlot();
};

PreloadAATableWorkSlot::PreloadAATableWorkSlot()
	: m_completedEvent(nullptr)
	, m_workerThread(nullptr)
	, m_spec(nullptr)
	, m_queued(false)
{
}

PreloadAATableWorkSlot::~PreloadAATableWorkSlot()
{
	if (m_workerThread)
		m_workerThread->Destroy();
}


void PreloadAATableThreadFunc(void *context)
{
	PreloadAATableWorkSlot *wSlot = static_cast<PreloadAATableWorkSlot*>(context);

	PortabilityLayer::StandardPalette *sp = PortabilityLayer::StandardPalette::GetInstance();

	if (wSlot->m_spec->m_isTone)
		sp->GetCachedToneAATable(wSlot->m_spec->m_color.r, wSlot->m_mutex);
	else
		sp->GetCachedPaletteAATable(wSlot->m_spec->m_color, wSlot->m_mutex);

	wSlot->m_singleJobCompleted.fetch_add(1, std::memory_order_release);
	wSlot->m_completedEvent->Signal();
}

void PreloadAATables()
{
	PortabilityLayer::StandardPalette *sp = PortabilityLayer::StandardPalette::GetInstance();
	PortabilityLayer::RGBAColor preloadColors[] =
	{
		PortabilityLayer::RGBAColor::Create(255, 255, 255, 255),
		PortabilityLayer::RGBAColor::Create(255, 51, 51, 255),
		PortabilityLayer::RGBAColor::Create(255, 0, 0, 255),
		PortabilityLayer::RGBAColor::Create(255, 255, 0, 255),
		PortabilityLayer::RGBAColor::Create(0, 255, 255, 255),
		PortabilityLayer::RGBAColor::Create(0, 0, 255, 255),
		PortabilityLayer::RGBAColor::Create(204, 102, 51, 255),
	};

	const size_t numPalettePreloads = sizeof(preloadColors) / sizeof(preloadColors[0]);
	const size_t maxTonePreloads = numPalettePreloads * 3;

	PreloadAATableSpec specs[numPalettePreloads + maxTonePreloads];
	for (size_t i = 0; i < numPalettePreloads; i++)
	{
		specs[i].m_color = preloadColors[i];
		specs[i].m_isTone = false;
	}

	size_t numTonePreloads = 0;
	for (size_t i = 0; i < numPalettePreloads; i++)
	{
		const uint8_t rgb[3] = { preloadColors[i].r, preloadColors[i].g, preloadColors[i].b };

		for (int ch = 0; ch < 3; ch++)
		{
			uint8_t tone = rgb[ch];

			bool toneAlreadyQueued = false;
			for (size_t j = 0; j < numTonePreloads; j++)
			{
				if (specs[numPalettePreloads + j].m_color.r == tone)
				{
					toneAlreadyQueued = true;
					break;
				}
			}

			if (!toneAlreadyQueued)
			{
				PreloadAATableSpec &spec = specs[i + numTonePreloads];
				numTonePreloads++;

				spec.m_color = PortabilityLayer::RGBAColor::Create(tone, tone, tone, 255);
				spec.m_isTone = true;
			}
		}
	}

	PortabilityLayer::MemoryManager *mm = PortabilityLayer::MemoryManager::GetInstance();

	const int numAASpecs = numPalettePreloads + numTonePreloads;

	unsigned int cpus = PLDrivers::GetSystemServices()->GetCPUCount();
	if (cpus < 2)
	{
		for (size_t i = 0; i < numAASpecs; i++)
		{
			PreloadAATableThreadFunc(specs + i);
			StepLoadScreen(1, false);
		}
	}
	else
	{
		cpus -= 1;

		int queuedSpecs = 0;
		int completedSpecs = 0;

		PreloadAATableWorkSlot *slots = static_cast<PreloadAATableWorkSlot*>(mm->Alloc(sizeof(PreloadAATableWorkSlot) * cpus));

		IGpMutex *mutex = PLDrivers::GetSystemServices()->CreateMutex();

		for (unsigned int i = 0; i < cpus; i++)
		{
			PreloadAATableWorkSlot *slot = new (slots + i) PreloadAATableWorkSlot();
			slot->m_workerThread = PortabilityLayer::WorkerThread::Create();
			slot->m_completedEvent = PLDrivers::GetSystemServices()->CreateThreadEvent(true, false);
			slot->m_mutex = mutex;
		}

		while (completedSpecs < numAASpecs)
		{
			int completedThisStep = 0;
			for (unsigned int i = 0; i < cpus; i++)
			{
				PreloadAATableWorkSlot &slot = slots[i];

				if (slot.m_queued)
				{
					if (slot.m_singleJobCompleted.load(std::memory_order_acquire) != 0)
					{
						slot.m_completedEvent->Wait();
						slot.m_queued = false;
						completedSpecs++;

						completedThisStep++;
					}
				}

				if (!slot.m_queued)
				{
					if (queuedSpecs < numAASpecs)
					{
						slot.m_queued = true;
						slot.m_spec = specs + queuedSpecs;
						slot.m_singleJobCompleted.store(0, std::memory_order_release);
						slot.m_workerThread->AsyncExecuteTask(PreloadAATableThreadFunc, &slot);

						queuedSpecs++;
					}
				}
			}

			if (completedThisStep > 0)
				StepLoadScreen(completedThisStep, false);

			StepLoadScreenRing();
			Delay(1, nullptr);
		}

		for (unsigned int i = 0; i < cpus; i++)
			slots[i].~PreloadAATableWorkSlot();

		mm->Release(slots);
		mutex->Destroy();
	}
}

void gpAppInit()
{
	// This is called before the display driver is initialized
	InstallResolutionHandler();
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

	if (thisMac.isTouchscreen)
		PortabilityLayer::MenuManager::GetInstance()->SetMenuTouchScreenStyle(true);

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

	InitLoadingWindow();	StepLoadScreen(2, true);
	PreloadAATables();
	assert(isPrefsLoaded || loadScreenWindowPhase == 0);
	PreloadFonts();		StepLoadScreen(2, true);

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
	VariableInit();						StepLoadScreen(2, true);
	GetExtraCursors();					StepLoadScreen(2, true);
	InitMarquee();
	CreatePointers();					StepLoadScreen(2, true);
	InitSrcRects();
	CreateOffscreens();					StepLoadScreen(2, true);

	if (loadScreenWindow)
	{
		PortabilityLayer::WindowManager::GetInstance()->FlickerWindowOut(loadScreenWindow, 32);
		PortabilityLayer::WindowManager::GetInstance()->DestroyWindow(loadScreenWindow);
		PLSysCalls::Sleep(15);
	}

	if (loadScreenRingSurface)
	{
		DisposeGWorld(loadScreenRingSurface);
		loadScreenRingSurface = nullptr;
	}

	if (!isPrefsLoaded)
	{
		WriteOutPrefs();
	}

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

	if (thisMac.isTouchscreen)
		StartMainMenuUI();

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

