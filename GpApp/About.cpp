//============================================================================
//----------------------------------------------------------------------------
//									About.c
//----------------------------------------------------------------------------
//============================================================================

#include "PLArrayView.h"
#include "PLKeyEncoding.h"
#include "PLControlDefinitions.h"
#include "FontFamily.h"
#include "PLButtonWidget.h"
#include "PLLabelWidget.h"
#include "PLNumberFormatting.h"
#include "PLResources.h"
#include "PLSound.h"
#include "PLPasStr.h"
#include "PLStandardColors.h"
#include "PLSysCalls.h"
#include "PLTimeTaggedVOSEvent.h"
#include "PLWidgets.h"
#include "About.h"
#include "DialogManager.h"
#include "DialogUtils.h"
#include "Environ.h"
#include "Externs.h"
#include "GpApplicationName.h"
#include "GpBuildVersion.h"
#include "HostSystemServices.h"
#include "RenderedFont.h"
#include "RenderedFontMetrics.h"
#include "ResolveCachingColor.h"
#include "ResourceManager.h"
#include "ScanlineMask.h"
#include "WindowDef.h"
#include "WindowManager.h"


static void HiLiteOkayButton (DrawSurface *surface);
static void UnHiLiteOkayButton (DrawSurface *surface);
static void UpdateMainPict (Dialog *);
static int16_t AboutFilter(Dialog *, const TimeTaggedVOSEvent *evt);
static int16_t AboutFrameworkFilter(Dialog *, const TimeTaggedVOSEvent *evt);


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
	short			hit;
	
	aboutDialog = PortabilityLayer::DialogManager::GetInstance()->LoadDialog(kAboutDialogID, PL_GetPutInFrontWindowPtr(), nullptr);
//	if (aboutDialog == nil)
//		RedAlert(kErrDialogDidntLoad);
	
	version = PortabilityLayer::ResourceManager::GetInstance()->GetAppResource('vers', 1).StaticCast<VersionRecord>();
	if (version != nil)
	{
		messagePtr = (**version).shortVersion + 1 + (**version).shortVersion[0];

		memcpy(&longVersion, messagePtr, ((UInt8)*messagePtr) + 1);
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
}

void DoAboutFramework (void)
{
#define			kAboutFrameworkDialogTemplateID			2000

	const Rect windowRect = Rect::Create(0, 0, 272, 450);

	PortabilityLayer::WindowDef wdef = PortabilityLayer::WindowDef::Create(windowRect, PortabilityLayer::WindowStyleFlags::kAlert, true, 0, 0, PSTR(""));

	PortabilityLayer::ResolveCachingColor blackColor = StdColors::Black();
	PortabilityLayer::RenderedFont *font = GetApplicationFont(12, PortabilityLayer::FontFamilyFlag_Bold, true);
	PortabilityLayer::RenderedFont *fontLight = GetApplicationFont(8, PortabilityLayer::FontFamilyFlag_None, true);

	int16_t verticalPoint = 16 + font->GetMetrics().m_ascent;
	int16_t horizontalOffset = 16;
	const int16_t spacing = 12;

	PortabilityLayer::DialogManager *dialogManager = PortabilityLayer::DialogManager::GetInstance();
	Dialog *dialog = dialogManager->LoadDialogFromTemplate(kAboutFrameworkDialogTemplateID, windowRect, true, false, 0, 0, PL_GetPutInFrontWindowPtr(), PSTR(""), nullptr);

#ifdef NDEBUG
	#define ABOUT_DIALOG_CONFIGURATION_TAG "Release"
#else
	#define ABOUT_DIALOG_CONFIGURATION_TAG "Debug"
#endif

	Window *window = dialog->GetWindow();

	DrawSurface *surface = window->GetDrawSurface();
	surface->DrawString(Point::Create(horizontalOffset, verticalPoint + spacing * 0), PSTR(GP_APPLICATION_NAME " " GP_APPLICATION_VERSION_STRING "   \xa9" GP_APPLICATION_COPYRIGHT_STRING), blackColor, font);
	
	surface->DrawString(Point::Create(horizontalOffset, verticalPoint + spacing * 2), PSTR(GP_APPLICATION_NAME " is an unoffical third-party port of Glider PRO\xaa."), blackColor, font);

	surface->DrawString(Point::Create(horizontalOffset, verticalPoint + spacing * 4), PSTR("This software is not maintained by, supported by, endorsed by, or"), blackColor, font);
	surface->DrawString(Point::Create(horizontalOffset, verticalPoint + spacing * 5), PSTR("otherwise associated with the authors and publishers of Glider PRO\xaa."), blackColor, font);

	surface->DrawString(Point::Create(horizontalOffset, verticalPoint + spacing * 7), PSTR("Please do not contact any of them regarding issues that you have"), blackColor, font);
	surface->DrawString(Point::Create(horizontalOffset, verticalPoint + spacing * 8), PSTR("with " GP_APPLICATION_NAME "."), blackColor, font);

	surface->DrawString(Point::Create(horizontalOffset, verticalPoint + spacing * 10), PSTR("If you would like to contribute to this project, visit:"), blackColor, font);
	surface->DrawString(Point::Create(horizontalOffset, verticalPoint + spacing * 11), PSTR(GP_APPLICATION_WEBSITE_STRING), blackColor, font);

	surface->DrawString(Point::Create(horizontalOffset, verticalPoint + spacing * 13), PSTR("To report a problem or request support, submit an issue via"), blackColor, font);
	surface->DrawString(Point::Create(horizontalOffset, verticalPoint + spacing * 14), PSTR("the website above."), blackColor, font);

	surface->DrawString(Point::Create(horizontalOffset, verticalPoint + spacing * 16), PSTR("For more information, please see the accompanying documentation."), blackColor, font);

	surface->DrawString(Point::Create(horizontalOffset, windowRect.bottom - 16), PSTR("Build: " __TIMESTAMP__ " " ABOUT_DIALOG_CONFIGURATION_TAG), blackColor, fontLight);

	DrawDefaultButton(dialog);

	int16_t hit = 0;

	do
	{
		hit = dialog->ExecuteModal(AboutFrameworkFilter);
	} while (hit != kOkayButton);

	dialog->Destroy();
}

//==============================================================  Static Functions
//--------------------------------------------------------------  HiLiteOkayButton
// Draws my pseudo-button to appear as though it is clicked on.

static void HiLiteOkayButton (DrawSurface *surface)
{
	#define		kOkayButtPICTHiLit		151		// res ID of unhilit button PICT
	THandle<BitmapImage>	thePict;
	
	if (!okayButtIsHiLit)
	{
		thePict = PortabilityLayer::ResourceManager::GetInstance()->GetAppResource('PICT', kOkayButtPICTHiLit).StaticCast<BitmapImage>();
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
	THandle<BitmapImage>	thePict;
	
	if (okayButtIsHiLit)
	{
		thePict = PortabilityLayer::ResourceManager::GetInstance()->GetAppResource('PICT', kOkayButtPICTNotHiLit).StaticCast<BitmapImage>();
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
	NumToString((long)(thisMac.fullScreen.right - thisMac.fullScreen.left), theStr2);
	PasStringConcat(theStr, theStr2);
	PasStringConcat(theStr, PSTR("x"));
	NumToString((long)(thisMac.fullScreen.bottom - thisMac.fullScreen.top), theStr2);
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

static int16_t AboutFilter(Dialog *dialog, const TimeTaggedVOSEvent *evt)
{
	bool		handledIt = false;
	int16_t		hit = -1;

	if (!evt)
		return -1;

	Window *window = dialog->GetWindow();
	DrawSurface *surface = window->GetDrawSurface();

	if (evt->IsKeyDownEvent())
	{
		switch (PackVOSKeyCode(evt->m_vosEvent.m_event.m_keyboardInputEvent))
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
	else if (evt->m_vosEvent.m_eventType == GpVOSEventTypes::kMouseInput)
	{
		const GpMouseInputEvent &mouseEvt = evt->m_vosEvent.m_event.m_mouseInputEvent;
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



//--------------------------------------------------------------  AboutFrameworkFilter
// Dialog filter for the About Framework dialog.

static int16_t AboutFrameworkFilter(Dialog *dialog, const TimeTaggedVOSEvent *evt)
{
	bool		handledIt = false;
	int16_t		hit = -1;

	if (!evt)
		return -1;

	Window *window = dialog->GetWindow();
	DrawSurface *surface = window->GetDrawSurface();

	if (evt->IsKeyDownEvent())
	{
		switch (PackVOSKeyCode(evt->m_vosEvent.m_event.m_keyboardInputEvent))
		{
		case PL_KEY_SPECIAL(kEnter):
		case PL_KEY_NUMPAD_SPECIAL(kEnter):
			dialog->GetItems()[kOkayButton - 1].GetWidget()->SetHighlightStyle(kControlButtonPart, true);
			PLSysCalls::Sleep(8);
			dialog->GetItems()[kOkayButton - 1].GetWidget()->SetHighlightStyle(kControlButtonPart, false);
			hit = kOkayButton;
			handledIt = true;
			break;

		default:
			handledIt = false;
			break;
		}
	}

	if (!handledIt)
		return -1;

	return hit;
}

