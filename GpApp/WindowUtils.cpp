
//============================================================================
//----------------------------------------------------------------------------
//								WindowUtils.c
//----------------------------------------------------------------------------
//============================================================================

#include "PLPasStr.h"
#include "Externs.h"
#include "Environ.h"
#include "PLStandardColors.h"
#include "RectUtils.h"
#include "WindowDef.h"
#include "WindowManager.h"

#define kFloatingKind		2048
#define kMessageWindowTall	48


WindowPtr	mssgWindow;


//==============================================================  Functions
//--------------------------------------------------------------  GetWindowTopLeft

// Returns the top left coordinate of the specified window.  Corrdinates…
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
		*left = static_cast<short>(theWindow->m_wmX);
		*top = static_cast<short>(theWindow->m_wmY);
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
		*bounds = theWindow->m_surface.m_port.GetRect() + upperLeft;
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
		GetWindowBounds(theWindow, kWindowContentRgn, bounds);
	}
}

//--------------------------------------------------------------  FlagWindowFloating
// Sets the specified window's windowKind field to my own kFloatingKind…
// variable.  This way I can examine a window later and determine if…
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

// Tests a specific window to see if it is supposed to "float" above all…
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
	CenterRectInRect(&placementRect, &thisMac.screen);

	const PortabilityLayer::WindowDef wdef = PortabilityLayer::WindowDef::Create(placementRect, windowStyle, false, 0, 0, title);

	PortabilityLayer::WindowManager *wm = PortabilityLayer::WindowManager::GetInstance();
	mssgWindow = wm->CreateWindow(wdef);
	wm->PutWindowBehind(mssgWindow, wm->GetPutInFrontSentinel());

	if (mssgWindow != nil)
	{
		ShowWindow(mssgWindow);

		DrawSurface *surface = mssgWindow->GetDrawSurface();

		surface->SetClipRect(mssgWindowRect);
		surface->SetForeColor(StdColors::Black());
		surface->SetBackColor(StdColors::White());
		
		surface->SetSystemFont(12, 0);
	}
}

//--------------------------------------------------------------  SetMessageWindowMessage

// For the above message window, this function displays a string of text…
// in the center of the window.

void SetMessageWindowMessage (StringPtr message)
{
	Rect		mssgWindowRect;
	
	if (mssgWindow != nil)
	{
		DrawSurface *surface = mssgWindow->GetDrawSurface();

		SetRect(&mssgWindowRect, 0, 0, 256, kMessageWindowTall);
		InsetRect(&mssgWindowRect, 16, 16);
		surface->SetForeColor(StdColors::White());
		surface->FillRect(mssgWindowRect);

		const Point textPoint = Point::Create(mssgWindowRect.left, mssgWindowRect.bottom - 6);
		surface->SetForeColor(StdColors::Black());
		surface->DrawString(textPoint, message, true);
	}
}

//--------------------------------------------------------------  CloseMessageWindow

// Closes the previously referred to "message window".

void CloseMessageWindow (void)
{
	CloseThisWindow(&mssgWindow);
}

//--------------------------------------------------------------  CloseThisWindow

// Given a specific window, this function will close it and set the window…
// pointer to null.

void CloseThisWindow (WindowPtr *theWindow)
{
	if (*theWindow != nil)
		PortabilityLayer::WindowManager::GetInstance()->DestroyWindow(*theWindow);
	*theWindow = nil;
}

