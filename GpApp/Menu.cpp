
//============================================================================
//----------------------------------------------------------------------------
//									Menu.c
//----------------------------------------------------------------------------
//============================================================================


//#include <Balloons.h>
#include "PLNumberFormatting.h"
#include "PLKeyEncoding.h"
#include "PLHacks.h"
#include "PLMenus.h"
#include "PLPasStr.h"
#include "PLToolUtils.h"
#include "DialogManager.h"
#include "DialogUtils.h"
#include "Externs.h"
#include "House.h"
#include "MenuManager.h"
#include "ObjectEdit.h"


#define kSheWantsNewGame		1
#define kSheWantsResumeGame		2


void UpdateMenusEditMode (void);
void UpdateMenusNonEditMode (void);
void UpdateMenusHouseOpen (void);
void UpdateMenusHouseClosed (void);
void UpdateResumeDialog (Dialog *);
Boolean ResumeFilter (Dialog *, EventRecord *, short *);
short QueryResumeGame (void);
void HeyYourPissingAHighScore (void);


MenuHandle	appleMenu, gameMenu, optionsMenu, houseMenu;
Boolean		menusUp, resumedSavedGame;


extern	long		incrementModeTime;
extern	short		demoHouseIndex, wasHouseVersion;
extern	short		splashOriginH, splashOriginV, numberRooms;
extern	Boolean		quitting, noRoomAtAll, twoPlayerGame;
extern	Boolean		isMapOpen, isToolsOpen, isPlayMusicIdle;
extern	Boolean		isCoordOpen, failedMusic, splashDrawn;
extern	Boolean		houseOpen;


//==============================================================  Functions
//--------------------------------------------------------------  UpdateMenusEditMode
// Sets the menus to reflect that user is in edit mode.

void UpdateMenusEditMode (void)
{
	DisableMenuItem(gameMenu, iNewGame);
	DisableMenuItem(gameMenu, iTwoPlayer);
	DisableMenuItem(gameMenu, iOpenSavedGame);
	DisableMenuItem(optionsMenu, iHighScores);
	DisableMenuItem(optionsMenu, iHelp);
	CheckMenuItem(optionsMenu, iEditor, true);
}

//--------------------------------------------------------------  UpdateMenusNonEditMode
// Sets the menus to reflect that user is NOT in edit mode.

void UpdateMenusNonEditMode (void)
{
	if ((noRoomAtAll) || (!houseOpen) || (numberRooms <= 0))
	{
		DisableMenuItem(gameMenu, iNewGame);
		DisableMenuItem(gameMenu, iTwoPlayer);
		DisableMenuItem(gameMenu, iOpenSavedGame);
		if (houseOpen)
		{
			EnableMenuItem(gameMenu, iLoadHouse);
			EnableMenuItem(optionsMenu, iHighScores);
		}
		else
		{
			DisableMenuItem(gameMenu, iLoadHouse);
			DisableMenuItem(optionsMenu, iHighScores);
		}
	}
	else
	{
		EnableMenuItem(gameMenu, iNewGame);
		EnableMenuItem(gameMenu, iTwoPlayer);
		EnableMenuItem(gameMenu, iOpenSavedGame);
		EnableMenuItem(gameMenu, iLoadHouse);
		EnableMenuItem(optionsMenu, iHighScores);
	}
	if (demoHouseIndex == -1)
		DisableMenuItem(optionsMenu, iHelp);
	else
		EnableMenuItem(optionsMenu, iHelp);
	CheckMenuItem(optionsMenu, iEditor, false);
}

//--------------------------------------------------------------  UpdateMenusHouseOpen
// Sets the menus to reflect that a house is currently open.

void UpdateMenusHouseOpen (void)
{
	EnableMenuItem(gameMenu, iLoadHouse);
	if ((fileDirty) && (houseUnlocked))
		EnableMenuItem(houseMenu, iSave);
	else
		DisableMenuItem(houseMenu, iSave);
	if (houseUnlocked)
	{
		// EnableMenuItem(houseMenu, iSaveAs);
		EnableMenuItem(houseMenu, iHouse);
	}
	else
	{
		// DisableMenuItem(houseMenu, iSaveAs);
		DisableMenuItem(houseMenu, iHouse);
	}
	if ((noRoomAtAll) || (!houseUnlocked))
		DisableMenuItem(houseMenu, iRoom);
	else
		EnableMenuItem(houseMenu, iRoom);
	if ((objActive == kNoObjectSelected) || (!houseUnlocked))
	{
		DisableMenuItem(houseMenu, iObject);
		DisableMenuItem(houseMenu, iBringForward);
		DisableMenuItem(houseMenu, iSendBack);
	}
	else
	{
		EnableMenuItem(houseMenu, iObject);
		if ((objActive == kInitialGliderSelected) || 
				(objActive == kLeftGliderSelected) ||
				(objActive == kRightGliderSelected))
		{
			DisableMenuItem(houseMenu, iBringForward);
			DisableMenuItem(houseMenu, iSendBack);
		}
		else
		{
			EnableMenuItem(houseMenu, iBringForward);
			EnableMenuItem(houseMenu, iSendBack);
		}
	}
}

//--------------------------------------------------------------  UpdateMenusHouseClosed
// Sets the menus to reflect that a house is NOT currently open.

void UpdateMenusHouseClosed (void)
{
	DisableMenuItem(gameMenu, iLoadHouse);
	DisableMenuItem(houseMenu, iSave);
	// DisableMenuItem(houseMenu, iSaveAs);
	DisableMenuItem(houseMenu, iHouse);
	DisableMenuItem(houseMenu, iRoom);
	DisableMenuItem(houseMenu, iObject);
	DisableMenuItem(houseMenu, iCut);
	DisableMenuItem(houseMenu, iCopy);
	DisableMenuItem(houseMenu, iPaste);
	DisableMenuItem(houseMenu, iClear);
	DisableMenuItem(houseMenu, iDuplicate);
}

//--------------------------------------------------------------  UpdateClipboardMenus
// Set the Cut/Copy/Paste menus to reflect if we have data in the…
// Mac's "clipboard" or not.

void UpdateClipboardMenus (void)
{
	Str255		title;
	
	if (!houseOpen)
		return;

	PortabilityLayer::MenuManager *mm = PortabilityLayer::MenuManager::GetInstance();
	
	if (houseUnlocked)
	{
		if (objActive > kNoObjectSelected)
		{
			GetLocalizedString(36, title);
			mm->SetItemText(houseMenu, iCut - 1, title);
			GetLocalizedString(37, title);
			mm->SetItemText(houseMenu, iCopy - 1, title);
			GetLocalizedString(38, title);
			mm->SetItemText(houseMenu, iClear - 1, title);
			mm->SetItemEnabled(houseMenu, iDuplicate - 1, false);
		}
		else
		{
			GetLocalizedString(39, title);
			mm->SetItemText(houseMenu, iCut - 1, title);
			GetLocalizedString(40, title);
			mm->SetItemText(houseMenu, iCopy - 1, title);
			GetLocalizedString(41, title);
			mm->SetItemText(houseMenu, iClear - 1, title);
			mm->SetItemEnabled(houseMenu, iDuplicate - 1, false);
		}
		
		mm->SetItemEnabled(houseMenu, iCut - 1, true);
		mm->SetItemEnabled(houseMenu, iCopy - 1, true);
//		if (hasScrap)
//		{
//			EnableMenuItem(houseMenu, iPaste);
//			if (scrapIsARoom)
//			{
//				GetLocalizedString(42, title);
//				SetMenuItemText(houseMenu, iPaste, title);
//			}
//			else
//			{
//				GetLocalizedString(43, title);
//				SetMenuItemText(houseMenu, iPaste, title);
//			}
//		}
//		else
		{
			mm->SetItemEnabled(houseMenu, iPaste - 1, false);
			GetLocalizedString(44, title);
			mm->SetItemText(houseMenu, iPaste - 1, title);
		}
		mm->SetItemEnabled(houseMenu, iClear - 1, true);
		mm->SetItemEnabled(houseMenu, iGoToRoom - 1, true);
		mm->SetItemEnabled(houseMenu, iMapWindow - 1, true);
		mm->SetItemEnabled(houseMenu, iObjectWindow - 1, true);
		mm->SetItemEnabled(houseMenu, iCoordinateWindow - 1, true);
	}
	else
	{
		mm->SetItemEnabled(houseMenu, iCut - 1, false);
		mm->SetItemEnabled(houseMenu, iCopy - 1, false);
		mm->SetItemEnabled(houseMenu, iPaste - 1, false);
		mm->SetItemEnabled(houseMenu, iClear - 1, false);
		mm->SetItemEnabled(houseMenu, iDuplicate - 1, false);
		mm->SetItemEnabled(houseMenu, iGoToRoom - 1, false);
		mm->SetItemEnabled(houseMenu, iMapWindow - 1, false);
		mm->SetItemEnabled(houseMenu, iObjectWindow - 1, false);
		mm->SetItemEnabled(houseMenu, iCoordinateWindow - 1, false);
	}
}

//--------------------------------------------------------------  UpdateMenus
// Called whenever a significant change to the environment has taken…
// place and some of the menu states may have changes (for example,…
// a menui was grayed out before becuase it wasn't an option - now…
// perhaps the situation has changed and we want the menu to be "usable").

void UpdateMenus (Boolean newMode)
{
	if (!menusUp)
		return;
	
	if (newMode)
	{
		PortabilityLayer::MenuManager *mm = PortabilityLayer::MenuManager::GetInstance();
		if (theMode == kEditMode)
			InsertMenu(houseMenu, 0);
		else
		{
			THandle<Menu> houseMenu = mm->GetMenuByID(kHouseMenuID);
			if (houseMenu)
				mm->RemoveMenu(houseMenu);
		}
	}
	
	if (theMode == kEditMode)
	{
		UpdateMenusEditMode();
		if (houseOpen)
		{
			UpdateMenusHouseOpen();
			UpdateClipboardMenus();
		}
		else
			UpdateMenusHouseClosed();
		UpdateLinkControl();
	}
	else
		UpdateMenusNonEditMode();
}

//--------------------------------------------------------------  DoAppleMenu
// Handle the Apple menu (About box and desk accessories).

void DoAppleMenu (short theItem)
{
//	Str255		daName;
//	GrafPtr		wasPort;
//	short		daNumber;
	
	switch (theItem)
	{
		case iAbout:
		DoAbout();
		break;
		
		default:
//		GetMenuItemText(appleMenu, theItem, daName);
//		GetPort(&wasPort);
//		daNumber = OpenDeskAccesory(daName);
//		SetPort((GrafPtr)wasPort);
		break;
	}
}

//--------------------------------------------------------------  DoGameMenu
// Handle the user selecting an item from the Game menu.

void DoGameMenu (short theItem)
{
	switch (theItem)
	{
		case iNewGame:
		twoPlayerGame = false;
		resumedSavedGame = false;
		NewGame(kNewGameMode);
		break;
		
		case iTwoPlayer:
		twoPlayerGame = true;
		resumedSavedGame = false;
		NewGame(kNewGameMode);
		break;
		
		case iOpenSavedGame:
		resumedSavedGame = true;
		if (!IsHighScoreDisabled())
			HeyYourPissingAHighScore();
		if (OpenSavedGame())
		{
			twoPlayerGame = false;
			NewGame(kResumeGameMode);
		}
		break;
		
		case iLoadHouse:
#ifdef COMPILEDEMO
		DoNotInDemo();
#else
		if (splashDrawn)
		{
			DoLoadHouse();
			OpenCloseEditWindows();
			UpdateMenus(false);
			incrementModeTime = TickCount() + kIdleSplashTicks;
			if ((theMode == kSplashMode) || (theMode == kPlayMode))
			{
				Rect		updateRect;
				
				SetRect(&updateRect, splashOriginH + 474, splashOriginV + 304, splashOriginH + 474 + 166, splashOriginV + 304 + 12);
				UpdateMainWindow();
			}
		}
#endif
		break;
		
		case iQuit:
#ifndef COMPILEDEMO
		quitting = true;
		if (!QuerySaveChanges())
			quitting = false;
#else
		quitting = true;
#endif
		break;
		
		default:
		break;
	}
}

//--------------------------------------------------------------  DoOptionsMenu
// Handle the user selecting an item from the Options menu.

void DoOptionsMenu (short theItem)
{
#ifndef COMPILEDEMO
	PLError_t		theErr;
#endif
	
	switch (theItem)
	{
		case iEditor:
#ifdef COMPILEDEMO
		DoNotInDemo();
#else
		if (theMode == kEditMode)			// switching to splash mode
		{
			if (fileDirty)
				SortHouseObjects();
			if (!QuerySaveChanges())
				break;
			theMode = kSplashMode;
			CloseMapWindow();
			CloseToolsWindow();
			CloseCoordWindow();
			CloseLinkWindow();
			DeselectObject();
			StopMarquee();
			if (isPlayMusicIdle)
			{
				theErr = StartMusic();
				if (theErr != PLErrors::kNone)
				{
					YellowAlert(kYellowNoMusic, theErr);
					failedMusic = true;
				}
			}
			CloseMainWindow();
			OpenMainWindow();
			incrementModeTime = TickCount() + kIdleSplashTicks;
		}
		else if (theMode == kSplashMode)	// switching to edit mode
		{
			theMode = kEditMode;
			StopTheMusic();
			CloseMainWindow();
			OpenMainWindow();
			OpenCloseEditWindows();
		}
		InitCursor();
		UpdateMenus(true);
#endif
		break;
		
		case iHighScores:
		DoHighScores();
		incrementModeTime = TickCount() + kIdleSplashTicks;
		break;
		
		case iPrefs:
		DoSettingsMain();
		incrementModeTime = TickCount() + kIdleSplashTicks;
		break;
		
		case iHelp:
		DoDemoGame();
		break;
	}
}

//--------------------------------------------------------------  DoHouseMenu
// Handle the user selecting an item from the House menu (only in Edit mode).

void DoHouseMenu (short theItem)
{
#ifndef COMPILEDEMO
	short		direction, dist;
	Boolean		whoCares;
	
	switch (theItem)
	{
		case iNewHouse:
		if (CreateNewHouse())
		{
			whoCares = InitializeEmptyHouse();
			OpenCloseEditWindows();
		}
		break;
		
		case iSave:
		DeselectObject();
		if (fileDirty)
			SortHouseObjects();
		if ((fileDirty) && (houseUnlocked))
		{
//			SaveGame(false);
			if (wasHouseVersion < kHouseVersion)
				ConvertHouseVer1To2();
			wasHouseVersion = kHouseVersion;
			whoCares = WriteHouse(true);
			ForceThisRoom(thisRoomNumber);
			ReadyBackground(thisRoom->background, thisRoom->tiles);
			GetThisRoomsObjRects();
			DrawThisRoomsObjects();
		}
		break;
		
//		case iSaveAs:
//		whoCares = SaveHouseAs();
//		break;
		
		case iHouse:
		if (houseUnlocked)
			DoHouseInfo();
		break;
		
		case iRoom:
		if (houseUnlocked)
			DoRoomInfo();
		break;
		
		case iObject:
		if (houseUnlocked)
		{
			DoObjectInfo();
			if (ObjectHasHandle(&direction, &dist))
			{
				StartMarqueeHandled(&roomObjectRects[objActive], direction, dist);
				HandleBlowerGlider();
			}
			else
				StartMarquee(&roomObjectRects[objActive]);
		}
		break;
		
		case iCut:
		if (houseUnlocked)
		{
			if (objActive > kNoObjectSelected)
			{
//				PutObjectScrap();
				DeleteObject();
			}
			else
			{
//				PutRoomScrap();
				DeleteRoom(false);
			}
			UpdateClipboardMenus();
		}
		break;
		
		case iCopy:
		if (houseUnlocked)
		{
//			if (objActive > kNoObjectSelected)
//				PutObjectScrap();
//			else
//				PutRoomScrap();
			UpdateClipboardMenus();
		}
		break;
		
		case iPaste:
		if (houseUnlocked)
		{
/*			if (scrapIsARoom)
				GetRoomScrap();
			else
				GetObjectScrap();
			UpdateClipboardMenus();
*/
		}
		break;
		
		case iClear:
		if (houseUnlocked)
		{
			if (objActive > kNoObjectSelected)
				DeleteObject();
			else
				DeleteRoom(false);
			UpdateClipboardMenus();
		}
		break;
		
		case iDuplicate:
		if (houseUnlocked)
			DuplicateObject();
		break;
		
		case iBringForward:
		if (houseUnlocked)
			BringSendFrontBack(true);
		break;
		
		case iSendBack:
		if (houseUnlocked)
			BringSendFrontBack(false);
		break;
		
		case iGoToRoom:
		if (houseUnlocked)
			DoGoToDialog();
		break;
		
		case iMapWindow:
		if (houseUnlocked)
			ToggleMapWindow();
		break;
		
		case iObjectWindow:
		if (houseUnlocked)
			ToggleToolsWindow();
		break;
		
		case iCoordinateWindow:
		if (houseUnlocked)
			ToggleCoordinateWindow();
		break;
	}
#endif
}

//--------------------------------------------------------------  DoMenuChoice
// Users has selected a menu item - determin which menu was selected…
// and call the appropriate function above.

void DoMenuChoice (long menuChoice)
{
	short		theMenu, theItem;
	
	if (menuChoice == 0)
		return;
	
	theMenu = HiWord(menuChoice);
	theItem = LoWord(menuChoice);
	
	switch (theMenu)
	{
		case kAppleMenuID:
		DoAppleMenu(theItem);
		break;
		
		case kGameMenuID:
		DoGameMenu(theItem);
		break;
		
		case kOptionsMenuID:
		DoOptionsMenu(theItem);
		break;
		
		case kHouseMenuID:
		DoHouseMenu(theItem);
		break;
	}
}

//--------------------------------------------------------------  UpdateMapCheckmark
// Checks or unchecks the Map Window item (to indicate if open or not).

void UpdateMapCheckmark (Boolean checkIt)
{
	if (!menusUp)
		return;
	
	CheckMenuItem(houseMenu, iMapWindow, checkIt);
}

//--------------------------------------------------------------  UpdateToolsCheckmark
// Checks or unchecks the Tools Window item (to indicate if open or not).

void UpdateToolsCheckmark (Boolean checkIt)
{
	if (!menusUp)
		return;
	
	CheckMenuItem(houseMenu, iObjectWindow, checkIt);
}

//--------------------------------------------------------------  UpdateCoordinateCheckmark
// Checks or unchecks the Coordinates Window item (to indicate if open or not).

void UpdateCoordinateCheckmark (Boolean checkIt)
{
	if (!menusUp)
		return;
	
	CheckMenuItem(houseMenu, iCoordinateWindow, checkIt);
}

//--------------------------------------------------------------  UpdateResumeDialog
// Update function for Resume dialog (below).

void UpdateResumeDialog (Dialog *theDialog)
{
	DrawDialog(theDialog);
	DrawDefaultButton(theDialog);
}

//--------------------------------------------------------------  ResumeFilter
// Dialog filter for the Resume dialog (below).

Boolean ResumeFilter (Dialog *dial, EventRecord *event, short *item)
{
	switch (event->what)
	{
		case keyDown:
		switch (event->message)
		{
			case PL_KEY_SPECIAL(kEnter):
			case PL_KEY_NUMPAD_SPECIAL(kEnter):
			FlashDialogButton(dial, kOkayButton);
			*item = kOkayButton;
			return(true);
			break;
			
			default:
			return(false);
		}
		break;
		
		case updateEvt:
		if ((WindowPtr)event->message == dial->GetWindow())
		{
			SetPortDialogPort(dial);
			UpdateResumeDialog(dial);
			EndUpdate(dial->GetWindow());
			event->what = nullEvent;
		}
		return(false);
		break;
		
		default:
		return(false);
		break;
	}
}

//--------------------------------------------------------------  QueryResumeGame
// Dialog that asks user whether they want to resume a saved game or…
// begin a new one.  It displays a little info on the state of their…
// saved game (number of glider left, points, etc.).

short QueryResumeGame (void)
{
	#define			kResumeGameDial		1025
	Dialog			*theDial;
	houseType		*thisHousePtr;
	Str255			scoreStr, glidStr;
	long			hadPoints;
	short			hitWhat, hadGliders;
	char			wasState;
	Boolean			leaving;

	// get score & num. gliders
	thisHousePtr = *thisHouse;
	hadPoints = thisHousePtr->savedGame.score;
	hadGliders = thisHousePtr->savedGame.numGliders;
	NumToString(hadPoints, scoreStr);			// param text strings
	NumToString((long)hadGliders, glidStr);

	DialogTextSubstitutions substitutions;
	if (hadGliders == 1)
		substitutions = DialogTextSubstitutions(glidStr, PSTR(""), scoreStr);
	else
		substitutions = DialogTextSubstitutions(glidStr, PSTR("s"), scoreStr);
	
//	CenterDialog(kResumeGameDial);
	theDial = PortabilityLayer::DialogManager::GetInstance()->LoadDialog(kResumeGameDial, kPutInFront, &substitutions);
	if (theDial == nil)
		RedAlert(kErrDialogDidntLoad);
	SetPort((GrafPtr)theDial);
	
	ShowWindow(theDial->GetWindow());
	DrawDefaultButton(theDial);
	leaving = false;
	
	while (!leaving)
	{
		ModalDialog(ResumeFilter, &hitWhat);
		if ((hitWhat == kSheWantsNewGame) || (hitWhat == kSheWantsResumeGame))
		{
			leaving = true;
		}
	}
	theDial->Destroy();
	
	return (hitWhat);
}

//--------------------------------------------------------------  DoNotInDemo
// Only compiled for "demo version" of Glider PRO.  It brings up a…
// dialog that says, essentially, "x" feature is  not implemented in…
// the demo version.

#ifdef COMPILEDEMO
void DoNotInDemo (void)
{
	#define		kNotInDemoAlert		1037
	short		whoCares;
	
//	CenterAlert(kNotInDemoAlert);
	whoCares = PortabilityLayer::DialogManager::GetInstance()->DisplayAlert(kNotInDemoAlert);
}
#endif

//--------------------------------------------------------------  HeyYourPissingAHighScore

void HeyYourPissingAHighScore (void)
{
	#define		kNoHighScoreAlert	1046
	short		whoCares;
	
//	CenterAlert(kNoHighScoreAlert);
	
	whoCares = PortabilityLayer::DialogManager::GetInstance()->DisplayAlert(kNoHighScoreAlert, nullptr);
}

//--------------------------------------------------------------  OpenCloseEditWindows
// Function goes through and either closes or opens all the various…
// editing windows (in response to switching in or out of editor).

void OpenCloseEditWindows (void)
{
	if (theMode == kEditMode)
	{
		if (houseUnlocked)
		{
			if (isMapOpen)
				OpenMapWindow();
			if (isToolsOpen)
				OpenToolsWindow();
			if (isCoordOpen)
				OpenCoordWindow();
		}
		else
		{
			CloseMapWindow();
			CloseToolsWindow();
			CloseCoordWindow();
		}
	}
}

