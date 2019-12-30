//============================================================================
//----------------------------------------------------------------------------
//									About.c
//----------------------------------------------------------------------------
//============================================================================

#include "PLKeyEncoding.h"
#include "PLControlDefinitions.h"
#include "PLNumberFormatting.h"
#include "PLResources.h"
#include "PLSound.h"
#include "PLPasStr.h"
#include "About.h"
#include "DialogUtils.h"
#include "Environ.h"
#include "Externs.h"
#include "HostSystemServices.h"
#include "ScanlineMask.h"


static void HiLiteOkayButton (void);
static void UnHiLiteOkayButton (void);
static void UpdateMainPict (DialogPtr);
static Boolean AboutFilter (DialogPtr, EventRecord *theEvent, short *hit);


static PortabilityLayer::ScanlineMask	*okayButtScanlineMask;
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
	
	DialogPtr		aboutDialog;
	Str255			longVersion;
	StringPtr		messagePtr;
	VersRecHndl		version;
	ControlHandle	itemHandle;
	short			itemType, hit, wasResFile;
	ModalFilterUPP	aboutFilterUPP;
	
	aboutFilterUPP = NewModalFilterUPP(AboutFilter);
	
	wasResFile = CurResFile();
	UseResFile(thisMac.thisResFile);
	
	aboutDialog = GetNewDialog(kAboutDialogID, nil, (WindowRef)-1L);
//	if (aboutDialog == nil)
//		RedAlert(kErrDialogDidntLoad);
	
	version = GetResource('vers', 1).StaticCast<VersionRecord>();
	if (version != nil)
	{
		messagePtr = (**version).shortVersion + 1 + (**version).shortVersion[0];

		BlockMove((Ptr)messagePtr, &longVersion, ((UInt8)*messagePtr) + 1);
		SetDialogString(aboutDialog, kTextItemVers, longVersion);
	}
	
	GetDialogItem(aboutDialog, kOkayButton, &itemType, &itemHandle, &okayButtonBounds);
#if 0
	PL_NotYetImplemented_TODO("Misc");
	okayButtRgn = NewRgn();					// Create diagonal button region
	OpenRgn();
	MoveTo(okayButtonBounds.left + 1, okayButtonBounds.top + 45);
	Line(44, -44);							// These lines define the region
	Line(16, 16);
	Line(-44, 44);
	Line(-16, -16);
	CloseRgn(okayButtRgn);
#endif
	okayButtIsHiLit = false;				// Initially, button is not hilit
	clickedDownInOkay = false;				// Initially, didn't click in okay button
	GetDialogItem(aboutDialog, kPictItemMain, &itemType, &itemHandle, &mainPICTBounds);
	
	do										// Loop until user wants to exit
	{
		ModalDialog(aboutFilterUPP, &hit);
	}
	while ((hit != kOkayButton) && (okayButtScanlineMask != nil));
	
	if (okayButtScanlineMask != nil)
		okayButtScanlineMask->Destroy();			// Clean up!
	DisposeDialog(aboutDialog);
	DisposeModalFilterUPP(aboutFilterUPP);
	
	UseResFile(wasResFile);
}

//==============================================================  Static Functions
//--------------------------------------------------------------  HiLiteOkayButton
// Draws my pseudo-button to appear as though it is clicked on.

static void HiLiteOkayButton (void)
{
	#define		kOkayButtPICTHiLit		151		// res ID of unhilit button PICT
	PicHandle	thePict;
	
	if (!okayButtIsHiLit)
	{
		thePict = GetPicture(kOkayButtPICTHiLit);
		if (thePict != nil)
		{
			DrawPicture(thePict, &okayButtonBounds);
			thePict.Dispose();
			
			okayButtIsHiLit = true;
		}
	}
}

//--------------------------------------------------------------  UnHiLiteOkayButton

// Draws my pseudo-button normal (not clicked on).

static void UnHiLiteOkayButton (void)
{
	#define		kOkayButtPICTNotHiLit	150		// res ID of hilit button PICT
	PicHandle	thePict;
	
	if (okayButtIsHiLit)
	{
		thePict = GetPicture(kOkayButtPICTNotHiLit);
		if (thePict != nil)
		{
			DrawPicture(thePict, &okayButtonBounds);
			thePict.Dispose();
			
			okayButtIsHiLit = false;
		}
	}
}

//--------------------------------------------------------------  UpdateMainPict
// Redraws the main graphic in the dialog (in response to an update event).

static void UpdateMainPict (DialogPtr theDial)
{
	Str255		theStr, theStr2;
	uint64_t	freeMemory;
	
	DrawDialog(theDial);

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

//--------------------------------------------------------------  AboutFilter
// Dialog filter for the About dialog.

static Boolean AboutFilter (DialogPtr theDial, EventRecord *theEvent, short *hit)
{
	Point		mousePt;
	UInt32		dummyLong;
	Boolean		handledIt;
	
	if (Button() && clickedDownInOkay)
	{
		GetMouse(&mousePt);
		if(PointInScanlineMask(mousePt, okayButtScanlineMask))
			HiLiteOkayButton();
		else
			UnHiLiteOkayButton();
	}
	
	switch (theEvent->what)
	{
		case keyDown:
		switch (theEvent->message)
		{
			case PL_KEY_SPECIAL(kEnter):
			case PL_KEY_NUMPAD_SPECIAL(kEnter):
			HiLiteOkayButton();
			Delay(8, &dummyLong);
			UnHiLiteOkayButton();
			*hit = kOkayButton;
			handledIt = true;
			break;
			
			default:
			handledIt = false;
		}
		break;
		
		case mouseDown:
		mousePt = theEvent->where;
		GlobalToLocal(&mousePt);
		if(PointInScanlineMask(mousePt, okayButtScanlineMask))
		{
			clickedDownInOkay = true;
			handledIt = false;
		}
		else
			handledIt = false;
		break;
		
		case mouseUp:
		mousePt = theEvent->where;
		GlobalToLocal(&mousePt);
		if(PointInScanlineMask(mousePt, okayButtScanlineMask) && clickedDownInOkay)
		{
			UnHiLiteOkayButton();
			*hit = kOkayButton;
			handledIt = true;
		}
		else
		{
			clickedDownInOkay = false;
			handledIt = false;
		}
		break;
		
		case updateEvt:
		if ((WindowPtr)theEvent->message == mainWindow)
		{
			SetPort((GrafPtr)mainWindow);
			BeginUpdate((WindowPtr)theEvent->message);
			UpdateMainWindow();
			EndUpdate((WindowPtr)theEvent->message);
			SetPortDialogPort(theDial);
			handledIt = true;
		}
		else if ((WindowPtr)theEvent->message == (WindowPtr)theDial)
		{
			SetPortDialogPort(theDial);
			BeginUpdate((WindowPtr)theEvent->message);
			UpdateMainPict(theDial);
			EndUpdate((WindowPtr)theEvent->message);
			handledIt = false;
		}
		break;
		
		default:
		handledIt = false;
		break;
	}
	
	return (handledIt);
}

