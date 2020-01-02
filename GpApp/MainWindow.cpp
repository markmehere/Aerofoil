//============================================================================
//----------------------------------------------------------------------------
//								MainWindow.c
//----------------------------------------------------------------------------
//============================================================================


#include "PLNumberFormatting.h"
#include "PLToolUtils.h"
#include "PLPasStr.h"
#include "Externs.h"
#include "Environ.h"
#include "FontFamily.h"
#include "House.h"
#include "MenuManager.h"
#include "RectUtils.h"
#include "PLKeyEncoding.h"
#include "PLStandardColors.h"
#include "WindowDef.h"
#include "WindowManager.h"


#define kMainWindowID			128
#define kEditWindowID			129
#define kMenuWindowID			130


void DrawOnSplash (DrawSurface *surface);
void SetPaletteToGrays (void);
void HardDrawMainWindow (void);


CTabHandle		theCTab;
PixMapHandle	thePMap;
ColorSpec *		wasColors;
ColorSpec *		newColors;
CursHandle		handCursorH, vertCursorH, horiCursorH;
CursHandle		diagCursorH;
Cursor			handCursor, vertCursor, horiCursor;
Cursor			diagCursor;
Rect			workSrcRect;
DrawSurface		*workSrcMap;
Rect			mainWindowRect;
WindowPtr		mainWindow, menuWindow, boardWindow;
short			isEditH, isEditV;
short			playOriginH, playOriginV;
short			splashOriginH, splashOriginV;
short			theMode;
Boolean			fadeGraysOut, isDoColorFade, splashDrawn;

extern	short		toolSelected;
extern	Boolean		noRoomAtAll, isUseSecondScreen;
extern	Boolean		quickerTransitions, houseIsReadOnly;


//==============================================================  Functions
//--------------------------------------------------------------  DrawOnSplash

// Draws additional text on top of splash screen.

void DrawOnSplash(DrawSurface *surface)
{
	Str255		houseLoadedStr;
	
	PasStringCopy(PSTR("House: "), houseLoadedStr);
	PasStringConcat(houseLoadedStr, thisHouseName);
	if ((thisMac.hasQT) && (hasMovie))
		PasStringConcat(houseLoadedStr, PSTR(" (QT)"));

	surface->SetApplicationFont(9, PortabilityLayer::FontFamilyFlag_Bold);

	const Point textPoint = Point::Create(splashOriginH + 436, splashOriginV + 314);
	if (thisMac.isDepth == 4)
	{
		surface->SetForeColor(PortabilityLayer::RGBAColor::Create(255, 255, 255, 255));
		surface->DrawString(textPoint, houseLoadedStr);
	}
	else
	{
		if (houseIsReadOnly)
			ColorText(surface, textPoint, houseLoadedStr, 5L);
		else
			ColorText(surface, textPoint, houseLoadedStr, 28L);
	}
	
	#if defined(powerc) || defined(__powerc)
	TextSize(12);
	TextFace(0);
	TextFont(systemFont);
	ForeColor(blackColor);
	MoveTo(splashOriginH + 5, splashOriginV + 457);
	DrawString("\pPowerPC Native!");
	ForeColor(whiteColor);
	MoveTo(splashOriginH + 4, splashOriginV + 456);
	DrawString("\pPowerPC Native!");
	ForeColor(blackColor);
	#endif
}

//--------------------------------------------------------------  RedrawSplashScreen

void RedrawSplashScreen (void)
{
	Rect		tempRect;
	DrawSurface	*surface = workSrcMap;

	surface->SetForeColor(StdColors::Black());
	surface->FillRect(workSrcRect);

	QSetRect(&tempRect, 0, 0, 640, 460);
	QOffsetRect(&tempRect, splashOriginH, splashOriginV);
	LoadScaledGraphic(surface, kSplash8BitPICT, &tempRect);
	DrawOnSplash(surface);

	SetPortWindowPort(mainWindow);

	CopyBits((BitMap *)*GetGWorldPixMap(workSrcMap),
		GetPortBitMapForCopyBits(GetWindowPort(mainWindow)),
		&workSrcRect, &workSrcRect, srcCopy);

//	if (quickerTransitions)
//		DissBitsChunky(&workSrcRect);
//	else
//		DissBits(&workSrcRect);
	CopyRectMainToWork(&workSrcRect);

	mainWindow->m_surface.m_port.SetDirty(PortabilityLayer::QDPortDirtyFlag_Contents);
	PortabilityLayer::MenuManager::GetInstance()->SetMenuVisible(true);
}

//--------------------------------------------------------------  UpdateMainWindow

// Redraws the main window (depends on mode were in - splash, editing, playing).

void UpdateMainWindow (void)
{
	Rect		tempRect;
	
	SetPortWindowPort(mainWindow);
	
	if (theMode == kEditMode)
	{
		PauseMarquee();
		CopyBits((BitMap *)*GetGWorldPixMap(workSrcMap),
				GetPortBitMapForCopyBits(GetWindowPort(mainWindow)), 
				&mainWindowRect, &mainWindowRect, srcCopy);
		ResumeMarquee();
	}
	else if ((theMode == kSplashMode) || (theMode == kPlayMode))
	{
		workSrcMap->FillRect(workSrcRect);
		QSetRect(&tempRect, 0, 0, 640, 460);
		QOffsetRect(&tempRect, splashOriginH, splashOriginV);
		LoadScaledGraphic(workSrcMap, kSplash8BitPICT, &tempRect);
		CopyBits((BitMap *)*GetGWorldPixMap(workSrcMap), 
				GetPortBitMapForCopyBits(GetWindowPort(mainWindow)), 
				&workSrcRect, &mainWindowRect, srcCopy);
		SetPortWindowPort(mainWindow);
		
		DrawOnSplash(mainWindow->GetDrawSurface());
	}

	mainWindow->m_surface.m_port.SetDirty(PortabilityLayer::QDPortDirtyFlag_Contents);
	
	splashDrawn = true;
}

//--------------------------------------------------------------  UpdateMenuBarWindow
// Ugly kludge to cover over the menu bar when playing game on 2nd monitor.

void UpdateMenuBarWindow (DrawSurface *surface)
{
	Rect		bounds;
	
	if (menuWindow == nil)
		return;
	
	GetLocalWindowRect(menuWindow, &bounds);

	surface->SetForeColor(StdColors::Black());
	surface->FillRect(bounds);
}

//--------------------------------------------------------------  OpenMainWindow
// Opens up the main window (how it does this depends on mode were in).

void OpenMainWindow (void)
{
//	long		wasSeed;
	short		whichRoom;
	
	if (mainWindow != nil)
	{
		YellowAlert(kYellowUnaccounted, 6);
		return;
	}
	
	if (theMode == kEditMode)
	{
		if (menuWindow != nil)
			PortabilityLayer::WindowManager::GetInstance()->DestroyWindow(menuWindow);
		menuWindow = nil;
		
		QSetRect(&mainWindowRect, 0, 0, 512, 322);
		mainWindow = GetNewCWindow(kEditWindowID, nil, kPutInFront);
		SizeWindow(mainWindow, mainWindowRect.right, 
				mainWindowRect.bottom, false);
		
		if (OptionKeyDown())
		{
			isEditH = 3;
			isEditV = 41;
		}
		MoveWindow(mainWindow, isEditH, isEditV, true);
		ShowWindow(mainWindow);

		DrawSurface *mainWindowSurface = mainWindow->GetDrawSurface();

		mainWindowSurface->SetClipRect(mainWindowRect);
		mainWindowSurface->SetForeColor(StdColors::Black());
		mainWindowSurface->SetBackColor(StdColors::White());
		
		whichRoom = GetFirstRoomNumber();
		CopyRoomToThisRoom(whichRoom);
		ReflectCurrentRoom(false);
	}
	else
	{
		if (menuWindow == nil)
		{
			menuWindow = GetNewCWindow(kMenuWindowID, nil, kPutInFront);
			SizeWindow(menuWindow, RectWide(&thisMac.screen), 20, false);
			MoveWindow(menuWindow, thisMac.screen.left, 
					thisMac.screen.top, true);
			ShowWindow(menuWindow);
		}
		if (boardWindow == nil)
		{
			PortabilityLayer::WindowManager *windowManager = PortabilityLayer::WindowManager::GetInstance();

			Rect scorebarRect = thisMac.screen;
			scorebarRect.bottom = scorebarRect.top + kScoreboardTall;

			PortabilityLayer::WindowDef windowDef = PortabilityLayer::WindowDef::Create(scorebarRect, 0, true, false, 0, 0, PSTR("Scoreboard"));
			boardWindow = windowManager->CreateWindow(windowDef);
			if (boardWindow != nil)
				windowManager->PutWindowBehind(boardWindow, PL_GetPutInFrontWindowPtr());
			else
				PL_NotYetImplemented_TODO("Errors");
		}
		mainWindowRect = thisMac.screen;
		ZeroRectCorner(&mainWindowRect);
		mainWindowRect.bottom -= 20;		// thisMac.menuHigh
		mainWindow = GetNewCWindow(kMainWindowID, nil, kPutInFront);
		SizeWindow(mainWindow, mainWindowRect.right - mainWindowRect.left, 
				mainWindowRect.bottom - mainWindowRect.top, false);
		MoveWindow(mainWindow, thisMac.screen.left, 
				thisMac.screen.top + 20, true);	// thisMac.menuHigh
		ShowWindow(mainWindow);
		SetPortWindowPort(mainWindow);

		DrawSurface *mainWindowSurface = mainWindow->GetDrawSurface();

		mainWindowSurface->SetClipRect(mainWindowRect);
//		CopyRgn(mainWindow->clipRgn, mainWindow->visRgn);
		mainWindowSurface->SetForeColor(StdColors::Black());
		mainWindowSurface->SetBackColor(StdColors::White());
		mainWindowSurface->FillRect(mainWindowRect);
		
		splashOriginH = ((thisMac.screen.right - thisMac.screen.left) - 640) / 2;
		if (splashOriginH < 0)
			splashOriginH = 0;
		splashOriginV = ((thisMac.screen.bottom - thisMac.screen.top) - 480) / 2;
		if (splashOriginV < 0)
			splashOriginV = 0;
		
		workSrcMap->FillRect(workSrcRect);
		LoadGraphic(workSrcMap, kSplash8BitPICT);
		
//		if ((fadeGraysOut) && (isDoColorFade))
//		{
//			wasSeed = ExtractCTSeed((CGrafPtr)mainWindow);
//			SetPortWindowPort(mainWindow);
//			SetPaletteToGrays();
//			HardDrawMainWindow();
//			fadeGraysOut = false;
//			ForceCTSeed((CGrafPtr)mainWindow, wasSeed);
//		}
		
		SetPortWindowPort(mainWindow);
	}

	CopyBits((BitMap *)*GetGWorldPixMap(workSrcMap),
		GetPortBitMapForCopyBits(GetWindowPort(mainWindow)),
		&mainWindowRect, &mainWindowRect, srcCopy);

	mainWindow->m_surface.m_port.SetDirty(PortabilityLayer::QDPortDirtyFlag_Contents);
}

//--------------------------------------------------------------  CloseMainWindow

// Closes the main window.

void CloseMainWindow (void)
{
	if (mainWindow != nil)
		PortabilityLayer::WindowManager::GetInstance()->DestroyWindow(mainWindow);
	mainWindow = nil;

	if (boardWindow != nil)
		PortabilityLayer::WindowManager::GetInstance()->DestroyWindow(boardWindow);
	boardWindow = nil;
}

//--------------------------------------------------------------  ZoomBetweenWindows
// Zooms from one window size to another.  Just for effect.

/*
#ifndef COMPILEDEMO
void ZoomBetweenWindows (void)
{
	Rect		aRect;
	short		h, v;
	
	if (theMode == kEditMode)
	{
		QSetRect(&aRect, 0, 0, 512, 342);
		QOffsetRect(&aRect, isEditH, isEditV);
		ZoomRectToRect(&(thisMac.screen), &aRect);
	}
	else
	{
		aRect = mainWindow->portRect;
		GetWindowLeftTop(mainWindow, &h, &v);
		QOffsetRect(&aRect, h, v);
		ZoomRectToRect(&aRect, &(thisMac.screen));
	}
}
#endif
*/

//--------------------------------------------------------------  UpdateEditWindowTitle
// Handles changing the title across the top of the main window.  OnlyÉ
// relevant when editing a house (room title displayed in window title).

#ifndef COMPILEDEMO
void UpdateEditWindowTitle (void)
{
	Str255		newTitle, tempStr;
	
	if (mainWindow == nil)
		return;
	
	PasStringCopy(thisHouseName, newTitle);
	PasStringConcat(newTitle, PSTR(" - "));
	if (noRoomAtAll)
		PasStringConcat(newTitle, PSTR("No rooms"));
	else if (houseUnlocked)
	{
		PasStringConcat(newTitle, thisRoom->name);
		PasStringConcat(newTitle, PSTR(" ("));
		NumToString((long)thisRoom->floor, tempStr);
		PasStringConcat(newTitle, tempStr);
		PasStringConcat(newTitle, PSTR(", "));
		NumToString((long)thisRoom->suite, tempStr);
		PasStringConcat(newTitle, tempStr);
		PasStringConcat(newTitle, PSTR(")"));
	}
	else
		PasStringConcat(newTitle, PSTR("House Locked"));
	SetWTitle(mainWindow, newTitle);
}
#endif

//--------------------------------------------------------------  HandleMainClick

// Handle a mouse click in the main window (relevant only when editing).

void HandleMainClick (Point wherePt, Boolean isDoubleClick)
{
	KeyDownStates		theseKeys;
	
	if ((theMode != kEditMode) || (mainWindow == nil) || 
			(!houseUnlocked))
		return;
	
	SetPortWindowPort(mainWindow);
	GlobalToLocal(&wherePt);

	DrawSurface *mainWindowSurface = mainWindow->GetDrawSurface();
	
	if (toolSelected == kSelectTool)
		DoSelectionClick(mainWindowSurface, wherePt, isDoubleClick);
	else
		DoNewObjectClick(wherePt);
	
	GetKeys(theseKeys);
	if (!BitTst(theseKeys, PL_KEY_EITHER_SPECIAL(kShift)))
	{
		EraseSelectedTool();
		SelectTool(kSelectTool);
	}
}

//--------------------------------------------------------------  ShowMenuBarOld
// Displays the menu bar (after having been hidden).
/*
void ShowMenuBarOld (void)
{
	Rect			theRect;
	GrafPtr			wasPort, tempPort;
	RgnHandle		worldRgn, menuBarRgn;
	
	if (LMGetMBarHeight() == 0)
	{
		GetPort(&wasPort);
		tempPort = (GrafPtr)NewPtrClear(sizeof(GrafPort));
		OpenPort(tempPort);
		SetPort((GrafPtr)tempPort);
		
		LMSetMBarHeight(thisMac.menuHigh);
		
		theRect = (**GetGrayRgn()).rgnBBox;
		UnionRect(&theRect, &qd.screenBits.bounds, &theRect);
		worldRgn = NewRgn();
		OpenRgn();
		FrameRoundRect(&theRect, 16, 16);
		CloseRgn(worldRgn);
		
		theRect = qd.screenBits.bounds;
		theRect.bottom = theRect.top + thisMac.menuHigh;
		menuBarRgn = NewRgn();
		RectRgn(menuBarRgn, &theRect);
		
		SectRgn(worldRgn, menuBarRgn, menuBarRgn);	//	/------------------\	
		DisposeRgn(worldRgn);						//	|__________________|	
		
		UnionRgn(tempPort->visRgn, menuBarRgn, tempPort->visRgn);
		DiffRgn(tempPort->visRgn, menuBarRgn, tempPort->visRgn);
		DisposeRgn(menuBarRgn);
		
		ClosePort(tempPort);
		SetPort((GrafPtr)wasPort);
		
		DrawMenuBar();
	}
}
*/
//--------------------------------------------------------------  HideMenuBarOld
// Hides the menu bar - completely erasing it from the screen.
/*
void HideMenuBarOld (void)
{
	Rect			theRect;
	RgnHandle		worldRgn, menuBarRgn;
	GrafPtr			wasPort, tempPort;
	
	if (LMGetMBarHeight() != 0)
	{
		GetPort(&wasPort);
		tempPort = (GrafPtr)NewPtrClear(sizeof(GrafPort));
		OpenPort(tempPort);
		SetPort((GrafPtr)tempPort);
		
		LMSetMBarHeight(0);
		
		theRect = (**GetGrayRgn()).rgnBBox;
		UnionRect(&theRect, &qd.screenBits.bounds, &theRect);
		worldRgn = NewRgn();
		OpenRgn();
		FrameRoundRect(&theRect, 16, 16);
		CloseRgn(worldRgn);
		
		theRect = qd.screenBits.bounds;
		theRect.bottom = theRect.top + thisMac.menuHigh;
		menuBarRgn = NewRgn();
		RectRgn(menuBarRgn, &theRect);
		
		SectRgn(worldRgn, menuBarRgn, menuBarRgn);	//	/------------------\	
		DisposeRgn(worldRgn);						//	|__________________|	
		
		UnionRgn(tempPort->visRgn, menuBarRgn, tempPort->visRgn);
		DisposeRgn(menuBarRgn);
		
		PaintRect(&theRect);
		
		ClosePort(tempPort);
		SetPort((GrafPtr)wasPort);
	}
}
*/
//--------------------------------------------------------------  SetPaletteToGrays

// Sets up a gray palette corresponding in luminance to the standard colorÉ
// palette.  This is to facilitate the gray->color fade when the game comes up.
/*
void SetPaletteToGrays (void)
{
	GDHandle	theDevice;
	long		longGray;
	short		i;
	char		wasState;
	
	wasState = HGetState((Handle)thisGDevice);
	HLock((Handle)thisGDevice);
	thePMap = (*thisGDevice)->gdPMap;
	HSetState((Handle)thisGDevice, wasState);
	
	theCTab = (*thePMap)->pmTable;
	wasColors = nil;
	wasColors = (ColorSpec*)NewPtr(sizeof(ColorSpec) * 256);
	if (wasColors == nil)
		RedAlert(kErrNoMemory);
	
	newColors = nil;
	newColors = (ColorSpec*)NewPtr(sizeof(ColorSpec) * 256);
	if (newColors == nil)
		RedAlert(kErrNoMemory);
	
	for (i = 0; i < 256; i++)
	{
		wasColors[i] = (*theCTab)->ctTable[i];
		newColors[i] = (*theCTab)->ctTable[i];
		
		if (i != 5)
		{
			longGray = ((long)newColors[i].rgb.red * 3L) / 10L + 
				((long)newColors[i].rgb.green * 6L) / 10L + 
				((long)newColors[i].rgb.blue * 1L) / 10L;
			
			newColors[i].rgb.red = (unsigned short)longGray;
			newColors[i].rgb.green = (unsigned short)longGray;
			newColors[i].rgb.blue = (unsigned short)longGray;
		}
	}
	
	theDevice = GetGDevice();
	SetGDevice(thisGDevice);
	SetEntries(0, 255, newColors);
	SetGDevice(theDevice);
}
*/
//--------------------------------------------------------------  HardDrawMainWindow
// Ignores the ToolBox - this function draws direct to screen in order toÉ
// circumvent the Toolbox's attempt to color-match to the current palette.
/*
void HardDrawMainWindow (void)
{
	PixMapHandle	pixMapH;
	Point			offsetPt;
	long			srcRowBytes, destRowBytes;
	long			src;
	long			dest;
	short			i, w;
	SInt8			mode;
	char			wasState;
	
	wasState = HGetState((Handle)thisGDevice);
	HLock((Handle)thisGDevice);
	pixMapH = (**thisGDevice).gdPMap;
	HSetState((Handle)thisGDevice, wasState);
	
	srcRowBytes = (long)((*(workSrcMap->portPixMap))->rowBytes & 0x7FFF);
	destRowBytes = (**pixMapH).rowBytes & 0x7FFF;
	src = (long)((*(workSrcMap->portPixMap))->baseAddr);
	dest = (long)((**pixMapH).baseAddr) + splashOriginH + 
			((splashOriginV + thisMac.menuHigh) * destRowBytes);
	
	offsetPt.h = 0;
	offsetPt.v = 0;
	ShieldCursor(&mainWindowRect, offsetPt);
	mode = true32b;
	SwapMMUMode(&mode);
	for (i = 0; i < 460; i++)
	{
		for (w = 0; w < 160; w++)
		{
			*(long *)dest = *(long *)src;
			dest += 4L;
			src += 4L;
		}
		src -= 640;
		dest -= 640;
		src += srcRowBytes;
		dest += destRowBytes;
	}
	SwapMMUMode(&mode);
	ShowCursor();
}
*/
//--------------------------------------------------------------  WashColorIn
// Slowly walks the palette from its gray luminance state to the full colorÉ
// palette.  In this way, color appears to slowly wash in.
/*
void WashColorIn (void)
{
	#define		kGray2ColorSteps	180
	GDHandle	theDevice;
	long		longDelta;
	short		i, c;
	
	theDevice = GetGDevice();
	SetGDevice(thisGDevice);
	
	for (i = 0; i < kGray2ColorSteps; i++)
	{
		for (c = 0; c < 256; c++)
		{
			if (c != 5)
			{
				longDelta = (((long)wasColors[c].rgb.red - 
						(long)newColors[c].rgb.red) / 
						(long)(kGray2ColorSteps - i)) + (long)newColors[c].rgb.red;
				newColors[c].rgb.red = (unsigned short)longDelta;
				
				longDelta = (((long)wasColors[c].rgb.green - 
						(long)newColors[c].rgb.green) / 
						(long)(kGray2ColorSteps - i)) + 
						(long)newColors[c].rgb.green;
				newColors[c].rgb.green = (unsigned short)longDelta;
				
				longDelta = (((long)wasColors[c].rgb.blue - 
						(long)newColors[c].rgb.blue) / 
						(long)(kGray2ColorSteps - i)) + 
						(long)newColors[c].rgb.blue;
				newColors[c].rgb.blue = (unsigned short)longDelta;
			}
		}
		SetEntries(0, 255, newColors);
		if (Button())
			break;
	}
	
	SetEntries(0, 255, wasColors);
	SetGDevice(theDevice);
	
	RestoreColorsSlam();
	
	if (wasColors != nil)
		DisposePtr((Ptr)wasColors);
	if (newColors != nil)
		DisposePtr((Ptr)newColors);
}
*/
