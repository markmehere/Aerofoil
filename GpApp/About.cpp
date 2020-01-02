//============================================================================
//----------------------------------------------------------------------------
//									About.c
//----------------------------------------------------------------------------
//============================================================================

#include "PLArrayView.h"
#include "PLKeyEncoding.h"
#include "PLControlDefinitions.h"
#include "PLNumberFormatting.h"
#include "PLResources.h"
#include "PLSound.h"
#include "PLPasStr.h"
#include "PLSysCalls.h"
#include "PLWidgets.h"
#include "About.h"
#include "DialogManager.h"
#include "DialogUtils.h"
#include "Environ.h"
#include "Externs.h"
#include "HostSystemServices.h"
#include "ScanlineMask.h"
#include "PLTimeTaggedVOSEvent.h"


static void HiLiteOkayButton (DrawSurface *surface);
static void UnHiLiteOkayButton (DrawSurface *surface);
static void UpdateMainPict (Dialog *);
static int16_t AboutFilter(Dialog *, const TimeTaggedVOSEvent &evt);


static Point			okayButtLowerV, okayButtUpperV;
static Rect				okayButtonBounds, mainPICTBounds;
static Boolean			okayButtIsHiLit, clickedDownInOkay;


//==============================================================  Functions
//--------------------------------------------------------------  DoAbout
// Brings up the About dialog box.

void DoAbout (void)
{
	#define			kAboutDialogID			150		// res ID of About dialog
	#define			kTextItemVers			2		// item number of version text
	#define			kPictItemMain			4		// item number of main PICT
	
	Dialog			*aboutDialog;
	Str255			longVersion;
	StringPtr		messagePtr;
	VersRecHndl		version;
	ControlHandle	itemHandle;
	short			hit, wasResFile;
	
	wasResFile = CurResFile();
	UseResFile(thisMac.thisResFile);
	
	aboutDialog = PortabilityLayer::DialogManager::GetInstance()->LoadDialog(kAboutDialogID, PL_GetPutInFrontWindowPtr());
//	if (aboutDialog == nil)
//		RedAlert(kErrDialogDidntLoad);
	
	version = GetResource('vers', 1).StaticCast<VersionRecord>();
	if (version != nil)
	{
		messagePtr = (**version).shortVersion + 1 + (**version).shortVersion[0];

		BlockMove((Ptr)messagePtr, &longVersion, ((UInt8)*messagePtr) + 1);
		SetDialogString(aboutDialog, kTextItemVers, longVersion);
	}

	okayButtonBounds = aboutDialog->GetItems()[kOkayButton - 1].GetWidget()->GetRect();

	okayButtUpperV = Point::Create(okayButtonBounds.left + 45, okayButtonBounds.top + 1);
	okayButtLowerV = Point::Create(okayButtUpperV.h - 28, okayButtUpperV.v + 60);

	okayButtIsHiLit = false;				// Initially, button is not hilit
	clickedDownInOkay = false;				// Initially, didn't click in okay button

	mainPICTBounds = aboutDialog->GetItems()[kPictItemMain - 1].GetWidget()->GetRect();

	UpdateMainPict(aboutDialog);
	
	do										// Loop until user wants to exit
	{
		hit = aboutDialog->ExecuteModal(AboutFilter);
	}
	while (hit != kOkayButton);

	aboutDialog->Destroy();
	
	UseResFile(wasResFile);
}

//==============================================================  Static Functions
//--------------------------------------------------------------  HiLiteOkayButton
// Draws my pseudo-button to appear as though it is clicked on.

static void HiLiteOkayButton (DrawSurface *surface)
{
	#define		kOkayButtPICTHiLit		151		// res ID of unhilit button PICT
	THandle<Picture>	thePict;
	
	if (!okayButtIsHiLit)
	{
		thePict = GetPicture(kOkayButtPICTHiLit);
		if (thePict != nil)
		{
			surface->DrawPicture(thePict, okayButtonBounds);
			thePict.Dispose();
			
			okayButtIsHiLit = true;
		}
	}
}

//--------------------------------------------------------------  UnHiLiteOkayButton

// Draws my pseudo-button normal (not clicked on).

static void UnHiLiteOkayButton (DrawSurface *surface)
{
	#define		kOkayButtPICTNotHiLit	150		// res ID of hilit button PICT
	THandle<Picture>	thePict;
	
	if (okayButtIsHiLit)
	{
		thePict = GetPicture(kOkayButtPICTNotHiLit);
		if (thePict != nil)
		{
			surface->DrawPicture(thePict, okayButtonBounds);
			thePict.Dispose();
			
			okayButtIsHiLit = false;
		}
	}
}

//--------------------------------------------------------------  UpdateMainPict
// Redraws the main graphic in the dialog (in response to an update event).

static void UpdateMainPict (Dialog *theDial)
{
	Str255		theStr, theStr2;
	uint64_t	freeMemory;

	freeMemory = PortabilityLayer::HostSystemServices::GetInstance()->GetFreeMemoryCosmetic();
	
	PasStringCopy(PSTR("Memory:   "), theStr);		// display free memory

	long totalSize = static_cast<long>(freeMemory / 1024);
	NumToString(totalSize, theStr2);
	PasStringConcat(theStr, theStr2);
	PasStringConcat(theStr, PSTR("K"));
	DrawDialogUserText2(theDial, 7, theStr);
	
	PasStringCopy(PSTR("Screen:   "), theStr);		// display screen size/depth
	NumToString((long)(thisMac.screen.right - thisMac.screen.left), theStr2);
	PasStringConcat(theStr, theStr2);
	PasStringConcat(theStr, PSTR("x"));
	NumToString((long)(thisMac.screen.bottom - thisMac.screen.top), theStr2);
	PasStringConcat(theStr, theStr2);
	PasStringConcat(theStr, PSTR("x"));
	NumToString((long)thisMac.isDepth, theStr2);
	PasStringConcat(theStr, theStr2);
	DrawDialogUserText2(theDial, 8, theStr);
}

static bool PointIsInDiagonalOkayButton(const Point &pt)
{
	const Point upperVPt = pt - okayButtUpperV;
	const Point lowerVPt = pt - okayButtLowerV;

	const bool edge1 = (upperVPt.h + upperVPt.v) >= 0;
	const bool edge2 = (-upperVPt.h + upperVPt.v) >= 0;
	const bool edge3 = (lowerVPt.h - lowerVPt.v) >= 0;
	const bool edge4 = (-lowerVPt.h - lowerVPt.v) >= 0;

	return edge1 && edge2 && edge3 && edge4;
}

//--------------------------------------------------------------  AboutFilter
// Dialog filter for the About dialog.

static int16_t AboutFilter(Dialog *dialog, const TimeTaggedVOSEvent &evt)
{
	bool		handledIt = false;
	int16_t		hit = -1;

	Window *window = dialog->GetWindow();
	DrawSurface *surface = window->GetDrawSurface();

	if (evt.IsKeyDownEvent())
	{
		switch (PackVOSKeyCode(evt.m_vosEvent.m_event.m_keyboardInputEvent))
		{
		case PL_KEY_SPECIAL(kEnter):
		case PL_KEY_NUMPAD_SPECIAL(kEnter):
			HiLiteOkayButton(surface);
			PLSysCalls::Sleep(8);
			UnHiLiteOkayButton(surface);
			hit = kOkayButton;
			handledIt = true;
			break;

		default:
			handledIt = false;
			break;
		}
	}
	else if (evt.m_vosEvent.m_eventType == GpVOSEventTypes::kMouseInput)
	{
		const GpMouseInputEvent &mouseEvt = evt.m_vosEvent.m_event.m_mouseInputEvent;
		const Point mousePt = window->MouseToLocal(mouseEvt);

		if (mouseEvt.m_eventType == GpMouseEventTypes::kDown)
		{
			if (PointIsInDiagonalOkayButton(mousePt))
			{
				HiLiteOkayButton(surface);
				clickedDownInOkay = true;
				handledIt = false;
			}
			else
				handledIt = false;
		}
		else if (mouseEvt.m_eventType == GpMouseEventTypes::kUp)
		{
			if (PointIsInDiagonalOkayButton(mousePt) && clickedDownInOkay)
			{
				UnHiLiteOkayButton(surface);
				hit = kOkayButton;
				handledIt = true;
			}
			else
			{
				clickedDownInOkay = false;
				handledIt = false;
			}
		}
		else if (mouseEvt.m_eventType == GpMouseEventTypes::kMove)
		{
			if (clickedDownInOkay)
			{
				if (PointIsInDiagonalOkayButton(mousePt))
					HiLiteOkayButton(surface);
				else
					UnHiLiteOkayButton(surface);
			}
		}
	}

	if (!handledIt)
		return -1;

	return hit;
}

