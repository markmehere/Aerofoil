
//============================================================================
//----------------------------------------------------------------------------
//								WindowUtils.c
//----------------------------------------------------------------------------
//============================================================================

#include "PLPasStr.h"
#include "Externs.h"
#include "Environ.h"
#include "FontFamily.h"
#include "PLStandardColors.h"
#include "RectUtils.h"
#include "ResolveCachingColor.h"
#include "WindowDef.h"
#include "WindowManager.h"

#define kFloatingKind		2048
#define kMessageWindowTall	48


WindowPtr	mssgWindow;
WindowPtr	mssgWindowExclusiveStack;


//==============================================================  Functions
//--------------------------------------------------------------  GetWindowTopLeft

// Returns the top left coordinate of the specified window.  CorrdinatesÉ
// are (of course) global (local coordinates would always be (0, 0)).

void GetWindowLeftTop (WindowPtr theWindow, short *left, short *top)
{
	if (theWindow == nullptr)
	{
		*left = 0;
		*top = 0;
	}
	else
	{
		const Point windowPos = theWindow->GetTopLeftCoord();
		*left = windowPos.h;
		*top = windowPos.v;
	}
}

//--------------------------------------------------------------  GetWindowRect

// Returns bounding rectangle of the specified window in global coords.

void GetWindowRect (WindowPtr theWindow, Rect *bounds)
{
	if (theWindow != nil)
	{
		Point upperLeft;
		GetWindowLeftTop(theWindow, &upperLeft.h, &upperLeft.v);
		*bounds = theWindow->GetSurfaceRect() + upperLeft;
	}
}

//--------------------------------------------------------------  GetLocalWindowRect
// Returns bounding rectangle of the specified window in local coords.
// (When you just need its width and height.)

void GetLocalWindowRect (WindowPtr theWindow, Rect *bounds)
{
	if (theWindow != nil)
	{
		SetPortWindowPort(theWindow);
		*bounds = theWindow->GetDrawSurface()->m_port.GetRect();
	}
}

//--------------------------------------------------------------  FlagWindowFloating
// Sets the specified window's windowKind field to my own kFloatingKindÉ
// variable.  This way I can examine a window later and determine ifÉ
// it's supposed to "float" above all other windows.
/*
void FlagWindowFloating (WindowPtr theWindow)
{
	if (theWindow != nil)
	{
		((WindowPeek)theWindow)->windowKind = kFloatingKind;
		BringToFront(theWindow);
	}
}

//--------------------------------------------------------------  IsWindowFloating

// Tests a specific window to see if it is supposed to "float" above allÉ
// other windows.

Boolean	IsWindowFloating (WindowPtr theWindow)
{
	if (theWindow != nil)
	{
		return (((WindowPeek)theWindow)->windowKind == kFloatingKind);
	}
	else
		return (false);
}
*/
//--------------------------------------------------------------  OpenMessageWindow
// Brings up a simple message window.  Nice sort of utility function.
// Anytime you want a small, quick message to come up, call this.

void OpenMessageWindow (const PLPasStr &title)
{
	const uint16_t windowStyle = PortabilityLayer::WindowStyleFlags::kTitleBar;

	Rect		mssgWindowRect;
	
	SetRect(&mssgWindowRect, 0, 0, 256, kMessageWindowTall);

	Rect placementRect = mssgWindowRect;
	CenterRectInRect(&placementRect, &thisMac.fullScreen);

	const PortabilityLayer::WindowDef wdef = PortabilityLayer::WindowDef::Create(placementRect, windowStyle, false, 0, 0, title);

	PortabilityLayer::WindowManager *wm = PortabilityLayer::WindowManager::GetInstance();
	mssgWindow = wm->CreateWindow(wdef);
	wm->PutWindowBehind(mssgWindow, wm->GetPutInFrontSentinel());

	if (mssgWindow != nil)
	{
		ShowWindow(mssgWindow);

		DrawSurface *surface = mssgWindow->GetDrawSurface();
	}

	mssgWindowExclusiveStack = mssgWindow;

	wm->SwapExclusiveWindow(mssgWindowExclusiveStack);	// Push exclusive window

	if (doZooms)
		wm->FlickerWindowIn(mssgWindow, 32);
}

//--------------------------------------------------------------  SetMessageWindowMessage

// For the above message window, this function displays a string of textÉ
// in the center of the window.

void SetMessageWindowMessage (StringPtr message, const PortabilityLayer::RGBAColor &color)
{
	Rect		mssgWindowRect;
	
	if (mssgWindow != nil)
	{
		DrawSurface *surface = mssgWindow->GetDrawSurface();

		PortabilityLayer::RenderedFont *sysFont = GetSystemFont(12, PortabilityLayer::FontFamilyFlag_None, true);

		SetRect(&mssgWindowRect, 0, 0, 256, kMessageWindowTall);
		InsetRect(&mssgWindowRect, 16, 16);

		PortabilityLayer::ResolveCachingColor whiteColor = StdColors::White();
		surface->FillRect(mssgWindowRect, whiteColor);

		const Point textPoint = Point::Create(mssgWindowRect.left, mssgWindowRect.bottom - 6);

		PortabilityLayer::ResolveCachingColor specifiedColor = color;
		surface->DrawString(textPoint, message, specifiedColor, sysFont);
	}
}

//--------------------------------------------------------------  CloseMessageWindow

// Closes the previously referred to "message window".

void CloseMessageWindow (void)
{
	PortabilityLayer::WindowManager *wm = PortabilityLayer::WindowManager::GetInstance();

	if (doZooms)
		wm->FlickerWindowOut(mssgWindow, 32);

	wm->SwapExclusiveWindow(mssgWindowExclusiveStack);	// Pop exclusive window
	assert(mssgWindowExclusiveStack == mssgWindow);

	mssgWindowExclusiveStack = nullptr;

	CloseThisWindow(&mssgWindow);
}

//--------------------------------------------------------------  CloseThisWindow

// Given a specific window, this function will close it and set the windowÉ
// pointer to null.

void CloseThisWindow (WindowPtr *theWindow)
{
	if (*theWindow != nil)
		PortabilityLayer::WindowManager::GetInstance()->DestroyWindow(*theWindow);
	*theWindow = nil;
}

