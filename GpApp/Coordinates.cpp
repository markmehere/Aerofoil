//============================================================================
//----------------------------------------------------------------------------
//								  Coordinates.c
//----------------------------------------------------------------------------
//============================================================================


#include "PLNumberFormatting.h"
#include "PLPasStr.h"
#include "PLStandardColors.h"
#include "Externs.h"
#include "Environ.h"
#include "Marquee.h"
#include "ObjectEdit.h"
#include "RectUtils.h"
#include "ResolveCachingColor.h"
#include "WindowDef.h"
#include "WindowManager.h"


Rect			coordWindowRect;
WindowPtr		coordWindow;
short			isCoordH, isCoordV;
short			coordH, coordV, coordD;
Boolean			isCoordOpen;


//==============================================================  Functions
//--------------------------------------------------------------  SetCoordinateHVD

// Given a horizontal, vertical and distance value, this functionÉ
// displays these values in the Coordinates window.

void SetCoordinateHVD (short h, short v, short d)
{
#ifndef COMPILEDEMO
	if (h != -2)
		coordH = h;
	if (v != -2)
		coordV = v;
	if (d != -2)
		coordD = d;
	UpdateCoordWindow();
#endif
}

//--------------------------------------------------------------  DeltaCoordinateD

// When the user is dragging a handle (such as the height of a blower)É
// this function can be called and passed the amount by which the userÉ
// has changed the height (delta).  This function then displays it inÉ
// the Coordinate window.

void DeltaCoordinateD (short d)
{
#ifndef COMPILEDEMO
	coordD = d;
	UpdateCoordWindow();
#endif
}

//--------------------------------------------------------------  UpdateCoordWindow

// Completely redraws and updates the Coordinate window.

void UpdateCoordWindow (void)
{
#ifndef COMPILEDEMO
	Str255		tempStr, numStr;
	
	if (coordWindow == nil)
		return;


	PortabilityLayer::ResolveCachingColor blackColor = StdColors::Black();
	PortabilityLayer::ResolveCachingColor whiteColor = StdColors::White();
	PortabilityLayer::ResolveCachingColor blueColor = StdColors::Blue();

	DrawSurface *surface = coordWindow->GetDrawSurface();

	surface->FillRect(coordWindowRect, whiteColor);

	PasStringCopy(PSTR("h: "), tempStr);
	if (coordH != -1)
	{
		NumToString((long)coordH, numStr);
		PasStringConcat(tempStr, numStr);
	}
	else
		PasStringConcat(tempStr, PSTR("-"));

	surface->DrawString(Point::Create(5, 12), tempStr, true, blackColor);
	
	PasStringCopy(PSTR("v: "), tempStr);
	if (coordV != -1)
	{
		NumToString((long)coordV, numStr);
		PasStringConcat(tempStr, numStr);
	}
	else
		PasStringConcat(tempStr, PSTR("-"));

	surface->DrawString(Point::Create(4, 22), tempStr, true, blackColor);
	
	PasStringCopy(PSTR("d: "), tempStr);
	if (coordD != -1)
	{
		NumToString((long)coordD, numStr);
		PasStringConcat(tempStr, numStr);
	}
	else
		PasStringConcat(tempStr, PSTR("-"));

	surface->DrawString(Point::Create(5, 32), tempStr, true, blueColor);
#endif
}

//--------------------------------------------------------------  OpenCoordWindow
// Brings up the Coordinate window.

void OpenCoordWindow (void)
{
#ifndef COMPILEDEMO
	Rect		src, dest;
	Point		globalMouse;
	short		direction, dist;

	PortabilityLayer::WindowManager *wm = PortabilityLayer::WindowManager::GetInstance();
	
	if (coordWindow == nil)
	{
		const uint16_t windowStyle = PortabilityLayer::WindowStyleFlags::kTitleBar | PortabilityLayer::WindowStyleFlags::kMiniBar | PortabilityLayer::WindowStyleFlags::kCloseBox;

		QSetRect(&coordWindowRect, 0, 0, 50, 38);

		{
			PortabilityLayer::WindowDef wdef = PortabilityLayer::WindowDef::Create(coordWindowRect, windowStyle, true, 0, 0, PSTR("Tools"));
			coordWindow = wm->CreateWindow(wdef);
		}
		
		if (coordWindow == nil)
			RedAlert(kErrNoMemory);
		
//		if (OptionKeyDown())
//		{
//			isCoordH = qd.screenBits.bounds.right - 55;
//			isCoordV = 204;
//		}
		MoveWindow(coordWindow, isCoordH, isCoordV, true);

		GetWindowRect(coordWindow, &dest);
		wm->PutWindowBehind(coordWindow, wm->GetPutInFrontSentinel());
		PortabilityLayer::WindowManager::GetInstance()->ShowWindow(coordWindow);
//		FlagWindowFloating(coordWindow);	TEMP - use flaoting windows
		
		coordH = -1;
		coordV = -1;
		coordD = -1;

		coordWindow->GetDrawSurface()->SetApplicationFont(9, 0);
		
		if (objActive != kNoObjectSelected)
		{
			if (ObjectHasHandle(&direction, &dist))
				coordD = dist;
			SetCoordinateHVD(theMarquee.bounds.left, theMarquee.bounds.top, coordD);
		}
	}
	
	UpdateCoordinateCheckmark(true);
#endif
}

//--------------------------------------------------------------  CloseCoordWindow
// Closes and disposes of the Coordinate window.

void CloseCoordWindow (void)
{
	CloseThisWindow(&coordWindow);
	UpdateCoordinateCheckmark(false);
}

//--------------------------------------------------------------  ToggleCoordinateWindow
// Toggles the Coordinate windows state between open and closed.

void ToggleCoordinateWindow (void)
{
#ifndef COMPILEDEMO
	if (coordWindow == nil)
	{
		OpenCoordWindow();
		isCoordOpen = true;
	}
	else
	{
		CloseCoordWindow();
		isCoordOpen = false;
	}
#endif
}

