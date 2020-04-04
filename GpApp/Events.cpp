
//============================================================================
//----------------------------------------------------------------------------
//									Events.c
//----------------------------------------------------------------------------
//============================================================================


#include "PLAppleEvents.h"
#include "PLKeyEncoding.h"
#include "PLTimeTaggedVOSEvent.h"
#include "PLToolUtils.h"
#include "PLQDraw.h"
#include "DialogManager.h"
#include "Externs.h"
#include "Environ.h"
#include "House.h"
#include "InputManager.h"
#include "ObjectEdit.h"
#include "WindowManager.h"


short BitchAboutColorDepth (void);
void HandleMouseEvent (const GpMouseInputEvent &, uint32_t);
void HandleKeyEvent (const KeyDownStates &keyStates, const GpKeyboardInputEvent &);
void HandleUpdateEvent (EventRecord *);
void HandleOSEvent (EventRecord *);
void HandleHighLevelEvent (EventRecord *);
void HandleIdleTask (void);
void IncrementMode (void);


long			lastUp, incrementModeTime;
UInt32			doubleTime;
Point			lastWhere;
short			idleMode;
Boolean			doAutoDemo, switchedOut;

extern	WindowPtr	mapWindow, toolsWindow, linkWindow;
extern	WindowPtr	menuWindow;
extern	short		isEditH, isEditV, isMapH, isMapV, isToolsH, isToolsV;
extern	short		isLinkH, isLinkV, isCoordH, isCoordV;
extern	Boolean		quitting, isMusicOn, failedMusic;
extern	Boolean		autoRoomEdit, newRoomNow, isPlayMusicIdle;


//==============================================================  Functions
//--------------------------------------------------------------  BitchAboutColorDepth
// Display a dialog that alerts the user that they have switched the bit…
// depth of the monitor under our noses.  They must return it to previous.

short BitchAboutColorDepth (void)
{
	#define		kColorSwitchedAlert		1042
	short		sheSaid;
	
//	CenterAlert(kColorSwitchedAlert);
	sheSaid = PortabilityLayer::DialogManager::GetInstance()->DisplayAlert(kColorSwitchedAlert, nullptr);
	
	return (sheSaid);
}

//--------------------------------------------------------------  HandleMouseEvent
// Handle a mouse click event.

void HandleMouseEvent (const GpMouseInputEvent &theEvent, uint32_t tick)
{
	WindowPtr	whichWindow;
	long		menuChoice, newSize;
	short		thePart, hDelta, vDelta;
	Boolean		isDoubleClick;
	Point		evtPoint = Point::Create(theEvent.m_x, theEvent.m_y);
	
	thePart = FindWindow(evtPoint, &whichWindow);
	
	switch (thePart)
	{
	case RegionIDs::kMenuBar:
		menuChoice = MenuSelect(evtPoint);
		DoMenuChoice(menuChoice);
		break;
		
	case RegionIDs::kTitleBar:
		PortabilityLayer::WindowManager::GetInstance()->DragWindow(whichWindow, evtPoint, thisMac.fullScreen);
		if (whichWindow == mainWindow)
			GetWindowLeftTop(whichWindow, &isEditH, &isEditV);
		else if (whichWindow == mapWindow)
			GetWindowLeftTop(whichWindow, &isMapH, &isMapV);
		else if (whichWindow == toolsWindow)
			GetWindowLeftTop(whichWindow, &isToolsH, &isToolsV);
		else if (whichWindow == linkWindow)
			GetWindowLeftTop(whichWindow, &isLinkH, &isLinkV);
		else if (whichWindow == coordWindow)
			GetWindowLeftTop(whichWindow, &isCoordH, &isCoordV);
		break;
		
	case RegionIDs::kClose:
		if (TrackGoAway(whichWindow, evtPoint))
		{
			if (whichWindow == mapWindow)
				ToggleMapWindow();
			else if (whichWindow == toolsWindow)
				ToggleToolsWindow();
			else if (whichWindow == linkWindow)
				CloseLinkWindow();
			else if (whichWindow == coordWindow)
				ToggleCoordinateWindow();
		}
		break;
		
	case RegionIDs::kExpand:
		if (whichWindow == mapWindow)
		{
			newSize = GrowWindow(mapWindow, evtPoint, &thisMac.gray);
			ResizeMapWindow(LoWord(newSize), HiWord(newSize));
		}
		break;
		
	case RegionIDs::kContent:
		if (whichWindow == mainWindow)
		{
			hDelta = evtPoint.h - lastWhere.h;
			if (hDelta < 0)
				hDelta = -hDelta;
			vDelta = evtPoint.v - lastWhere.v;
			if (vDelta < 0)
				vDelta = -vDelta;
			if (((tick - lastUp) < doubleTime) && (hDelta < 5) &&
					(vDelta < 5))
				isDoubleClick = true;
			else
			{
				isDoubleClick = false;
				lastUp = tick;
				lastWhere = evtPoint;
			}
			HandleMainClick(evtPoint, isDoubleClick);
		}
		else if (whichWindow == mapWindow)
			HandleMapClick(theEvent);
		else if (whichWindow == toolsWindow)
			HandleToolsClick(evtPoint);
		else if (whichWindow == linkWindow)
			HandleLinkClick(evtPoint);
		break;
		
	default:
		break;
	}
}

//--------------------------------------------------------------  HandleKeyEvent
// Handle a key-down event.

void HandleKeyEvent (const KeyDownStates &keyStates, const GpKeyboardInputEvent &theEvent)
{
	const intptr_t theChar = PackVOSKeyCode(theEvent);
	const bool shiftDown = keyStates.IsSet(PL_KEY_EITHER_SPECIAL(kShift));
	const bool commandDown = keyStates.IsSet(PL_KEY_EITHER_SPECIAL(kControl));
	const bool optionDown = keyStates.IsSet(PL_KEY_EITHER_SPECIAL(kAlt));
	
	if ((commandDown) && (!optionDown))
		DoMenuChoice(MenuKey(theChar));
	else 
	{
		switch (theChar)
		{
			case PL_KEY_SPECIAL(kPageUp):
			if (houseUnlocked)
				PrevToolMode();
			break;
			
			case PL_KEY_SPECIAL(kPageDown):
			if (houseUnlocked)
				NextToolMode();
			break;
			
#if BUILD_ARCADE_VERSION
			
			case PL_KEY_SPECIAL(kLeftArrow):
			DoOptionsMenu(iHighScores);
			break;
			
			case PL_KEY_SPECIAL(kRightArrow):
			DoOptionsMenu(iHelp);
			break;
			
			case PL_KEY_SPECIAL(kUpArrow):
			DoGameMenu(iNewGame);
			break;
			
			case PL_KEY_SPECIAL(kDownArrow):
			DoGameMenu(iNewGame);
			break;
			
#else
			
			case PL_KEY_SPECIAL(kLeftArrow):
			if (houseUnlocked)
			{
				if (objActive == kNoObjectSelected)
					SelectNeighborRoom(kRoomToLeft);
				else
					MoveObject(kBumpLeft, shiftDown);
			}
			break;
			
			case PL_KEY_SPECIAL(kRightArrow):
			if (houseUnlocked)
			{
				if (objActive == kNoObjectSelected)
					SelectNeighborRoom(kRoomToRight);
				else
					MoveObject(kBumpRight, shiftDown);
			}
			break;
			
			case PL_KEY_SPECIAL(kUpArrow):
			if (houseUnlocked)
			{
				if (objActive == kNoObjectSelected)
					SelectNeighborRoom(kRoomAbove);
				else
					MoveObject(kBumpUp, shiftDown);
			}
			break;
			
			case PL_KEY_SPECIAL(kDownArrow):
			if (houseUnlocked)
			{
				if (objActive == kNoObjectSelected)
					SelectNeighborRoom(kRoomBelow);
				else
					MoveObject(kBumpDown, shiftDown);
			}
			break;
			
#endif
			
			case PL_KEY_SPECIAL(kDelete):
			if (houseUnlocked)
			{
				if (objActive == kNoObjectSelected)
					DeleteRoom(true);
				else
					DeleteObject();
			}
			break;
			
			case PL_KEY_SPECIAL(kTab):
			if ((theMode == kEditMode) && (houseUnlocked))
			{
				if (shiftDown)
					SelectPrevObject();
				else
					SelectNextObject();
			}
			break;
			
			case PL_KEY_SPECIAL(kEscape):
			if ((theMode == kEditMode) && (houseUnlocked))
				DeselectObject();
			break;
			
			case PL_KEY_ASCII('A'):
			if ((theMode == kEditMode) && (houseUnlocked))
				SetSpecificToolMode(kApplianceMode);
			break;
			
			case PL_KEY_ASCII('B'):
			if ((theMode == kEditMode) && (houseUnlocked))
				SetSpecificToolMode(kBlowerMode);
			break;
			
			case PL_KEY_ASCII('C'):
			if ((theMode == kEditMode) && (houseUnlocked))
				SetSpecificToolMode(kClutterMode);
			break;
			
			case PL_KEY_ASCII('E'):
			if ((theMode == kEditMode) && (houseUnlocked))
				SetSpecificToolMode(kEnemyMode);
			break;
			
			case PL_KEY_ASCII('F'):
			if ((theMode == kEditMode) && (houseUnlocked))
				SetSpecificToolMode(kFurnitureMode);
			break;
			
			case PL_KEY_ASCII('L'):
			if ((theMode == kEditMode) && (houseUnlocked))
				SetSpecificToolMode(kLightMode);
			break;
			
			case PL_KEY_ASCII('P'):
			if ((theMode == kEditMode) && (houseUnlocked))
				SetSpecificToolMode(kBonusMode);
			break;
			
			case PL_KEY_ASCII('S'):
			if ((theMode == kEditMode) && (houseUnlocked))
				SetSpecificToolMode(kSwitchMode);
			break;
			
			case PL_KEY_ASCII('T'):
			if ((theMode == kEditMode) && (houseUnlocked))
				SetSpecificToolMode(kTransportMode);
			break;
			
			default:
			break;
		}
	}
}

//--------------------------------------------------------------  HandleUpdateEvent
// Handle an update event.

#if 0
void HandleUpdateEvent (EventRecord *theEvent)
{	
	if ((WindowPtr)theEvent->message == mainWindow)
	{
		SetPort((GrafPtr)mainWindow);
		UpdateMainWindow();
		EndUpdate(mainWindow);
	}
	else if ((WindowPtr)theEvent->message == mapWindow)
	{
		SetPort((GrafPtr)mapWindow);
		UpdateMapWindow();
		EndUpdate(mapWindow);
	}
	else if ((WindowPtr)theEvent->message == toolsWindow)
	{
		SetPort((GrafPtr)toolsWindow);
		UpdateToolsWindow();
		EndUpdate(toolsWindow);
	}
	else if ((WindowPtr)theEvent->message == linkWindow)
	{
		SetPort((GrafPtr)linkWindow);
		UpdateLinkWindow();
		EndUpdate(linkWindow);
	}
	else if ((WindowPtr)theEvent->message == coordWindow)
	{
		SetPort((GrafPtr)coordWindow);
		UpdateCoordWindow();
		EndUpdate(coordWindow);
	}
	else if ((WindowPtr)theEvent->message == menuWindow)
	{
		UpdateMenuBarWindow(menuWindow->GetDrawSurface());
		EndUpdate(menuWindow);
	}
}

//--------------------------------------------------------------  HandleOSEvent
// Handle an OS Event (MultiFinder - user has switched in or out of app).

void HandleOSEvent (EventRecord *theEvent)
{
	PLError_t		theErr;
	short		buttonHit;
	
	if (theEvent->message & 0x01000000)		// suspend or resume event
	{
		if (theEvent->message & 0x00000001)	// resume event
		{
			if (WhatsOurDepth() != thisMac.isDepth)
			{
				buttonHit = BitchAboutColorDepth();
				if (buttonHit == 1)			// player wants to Quit
				{
#ifndef COMPILEDEMO
					if (QuerySaveChanges())
						quitting = true;
#else
					quitting = true;
#endif
				}
				else
				{
					SwitchToDepth(thisMac.isDepth, thisMac.wasColorOrGray);
				}
			}
			switchedOut = false;
			InitCursor();
			if ((isPlayMusicIdle) && (theMode != kEditMode))
			{
				theErr = StartMusic();
				if (theErr != PLErrors::kNone)
				{
					YellowAlert(kYellowNoMusic, theErr);
					failedMusic = true;
				}
			}
			incrementModeTime = TickCount() + kIdleSplashTicks;
			
#ifndef COMPILEDEMO
//			if (theMode == kEditMode)
//				SeeIfValidScrapAvailable(true);
#endif
		}
		else								// suspend event
		{
			switchedOut = true;
			InitCursor();
			if ((isMusicOn) && (theMode != kEditMode))
				StopTheMusic();
		}
	}
}

//--------------------------------------------------------------  HandleHighLevelEvent
// Handle an AppleEvent (Open Document, Quit Application, etc.).

void HandleHighLevelEvent (EventRecord *theEvent)
{
	PLError_t		theErr;
	
	theErr = AEProcessAppleEvent(theEvent);
	if ((theErr != PLErrors::kNone) && (theErr != errAEEventNotHandled))
		YellowAlert(kYellowAppleEventErr, theErr);
}
#endif

//--------------------------------------------------------------  HandleIdleTask
// Handle some processing during event lulls.

void HandleIdleTask (void)
{
	if (theMode == kEditMode)
	{
		SetPort(&mainWindow->GetDrawSurface()->m_port);
		DoMarquee();
		
		if ((autoRoomEdit) && (newRoomNow))
		{
			if (theMode == kEditMode)
				DoRoomInfo();
			newRoomNow = false;
		}
	}
}

//--------------------------------------------------------------  HandleEvent
// "Master" function that tests for events and calls the above functions to…
// handle each event type.  Not called during and actual game.

void HandleEvent (void)
{
	TimeTaggedVOSEvent	theEvent;
	uint32_t			sleep = 2;
	bool				itHappened = true;

	const KeyDownStates *eventKeys = PortabilityLayer::InputManager::GetInstance()->GetKeys();
	if ((eventKeys->IsSet(PL_KEY_EITHER_SPECIAL(kControl))) &&
			(eventKeys->IsSet(PL_KEY_EITHER_SPECIAL(kAlt))))
	{
		HiliteAllObjects();
	}
	else if ((eventKeys->IsSet(PL_KEY_EITHER_SPECIAL(kAlt))) && (theMode == kEditMode) &&
			(houseUnlocked))
	{
		EraseSelectedTool();
		SelectTool(kSelectTool);
	}

	itHappened = WaitForEvent(&theEvent, sleep);
	
	if (itHappened)
	{
		if (theEvent.m_vosEvent.m_eventType == GpVOSEventTypes::kMouseInput)
		{
			switch (theEvent.m_vosEvent.m_event.m_mouseInputEvent.m_eventType)
			{
			case GpMouseEventTypes::kDown:
				HandleMouseEvent(theEvent.m_vosEvent.m_event.m_mouseInputEvent, theEvent.m_timestamp);
				break;
			default:
				break;
			}
		}
		else if (theEvent.m_vosEvent.m_eventType == GpVOSEventTypes::kKeyboardInput)
		{
			switch (theEvent.m_vosEvent.m_event.m_keyboardInputEvent.m_eventType)
			{
			case GpKeyboardInputEventTypes::kDown:
			case GpKeyboardInputEventTypes::kAuto:
				HandleKeyEvent(*eventKeys, theEvent.m_vosEvent.m_event.m_keyboardInputEvent);
				break;
			default:
				break;
			}
		}
	}
	else
		HandleIdleTask();
	
	if ((theMode == kSplashMode) && doAutoDemo && !switchedOut)
	{
		if (TickCount() >= incrementModeTime)
			DoDemoGame();
	}
}

//--------------------------------------------------------------  IgnoreThisClick

// Another inelegant kludge designed to temporarily prevent an unwanted…
// double-click to be registered.

void IgnoreThisClick (void)
{
	lastUp -= doubleTime;
	lastWhere.h = -100;
	lastWhere.v = -100;
}

