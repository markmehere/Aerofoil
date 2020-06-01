//============================================================================
//----------------------------------------------------------------------------
//									Input.c
//----------------------------------------------------------------------------
//============================================================================


#include "PLToolUtils.h"
#include "PLDialogs.h"
#include "PLKeyEncoding.h"
#include "DialogManager.h"
#include "Externs.h"
#include "InputManager.h"
#include "MainWindow.h"
#include "QDPixMap.h"
#include "RectUtils.h"


#define kNormalThrust		5
#define kHyperThrust		8
#define kHeliumLift			4
#define kEscPausePictID		1015
#define kTabPausePictID		1016
#define	kSavingGameDial		1042


void LogDemoKey (char);
void DoCommandKey (void);
void DoPause (void);
void DoBatteryEngaged (gliderPtr);
void DoHeliumEngaged (gliderPtr);
Boolean QuerySaveGame (void);


demoPtr		demoData;
Dialog		*saveDial;
short		demoIndex, batteryFrame;
Boolean		isEscPauseKey, paused, batteryWasEngaged;

extern	long		gameFrame;
extern	short		otherPlayerEscaped;
extern	Boolean		quitting, playing, onePlayerLeft, twoPlayerGame, demoGoing;


//==============================================================  Functions
//--------------------------------------------------------------  LogDemoKey

void LogDemoKey (char keyIs)
{
	demoData[demoIndex].frame = gameFrame;
	demoData[demoIndex].key = keyIs;
	demoIndex++;
}

//--------------------------------------------------------------  DoCommandKey

void DoCommandKey (void)
{
	const KeyDownStates *theKeys = PortabilityLayer::InputManager::GetInstance()->GetKeys();

	if (theKeys->IsSet(PL_KEY_ASCII('Q')))
	{
		playing = false;
		paused = false;
		if ((!twoPlayerGame) && (!demoGoing))
		{
			if (QuerySaveGame())
				SaveGame2();		// New save game.
		}
	}
	else if ((theKeys->IsSet(PL_KEY_ASCII('S'))) && (!twoPlayerGame))
	{
		RefreshScoreboard(kSavingTitleMode);
		SaveGame2();				// New save game.
		HideCursor();
		CopyRectWorkToMain(&workSrcRect);
		RefreshScoreboard(kNormalTitleMode);
	}
}

//--------------------------------------------------------------  DoPause

void DoPause (void)
{
	Rect		bounds;

	DrawSurface *surface = mainWindow->GetDrawSurface();

	QSetRect(&bounds, 0, 0, 214, 54);
	CenterRectInRect(&bounds, &houseRect);
	if (isEscPauseKey)
		LoadScaledGraphic(surface, kEscPausePictID, &bounds);
	else
		LoadScaledGraphic(surface, kTabPausePictID, &bounds);

	const KeyDownStates *theKeys = nullptr;

	do
	{
		theKeys = PortabilityLayer::InputManager::GetInstance()->GetKeys();

		Delay(1, nullptr);
	}
	while ((isEscPauseKey && theKeys->IsSet(PL_KEY_SPECIAL(kEscape))) || 
			(!isEscPauseKey && theKeys->IsSet(PL_KEY_SPECIAL(kTab))));
	
	paused = true;
	while (paused)
	{
		theKeys = PortabilityLayer::InputManager::GetInstance()->GetKeys();

		if ((isEscPauseKey && theKeys->IsSet(PL_KEY_SPECIAL(kEscape))) ||
				(!isEscPauseKey && theKeys->IsSet(PL_KEY_SPECIAL(kTab))))
			paused = false;
		else if (theKeys->IsSet(PL_KEY_EITHER_SPECIAL(kControl)))
			DoCommandKey();

		Delay(1, nullptr);
	}
	
	CopyBits((BitMap *)*GetGWorldPixMap(workSrcMap), 
			GetPortBitMapForCopyBits(mainWindow->GetDrawSurface()),
			&bounds, &bounds, srcCopy);
	
	do
	{
		theKeys = PortabilityLayer::InputManager::GetInstance()->GetKeys();
		Delay(1, nullptr);
	}
	while ((isEscPauseKey && theKeys->IsSet(PL_KEY_SPECIAL(kEscape))) ||
			(!isEscPauseKey && theKeys->IsSet(PL_KEY_SPECIAL(kTab))));
}

//--------------------------------------------------------------  DoBatteryEngaged

void DoBatteryEngaged (gliderPtr thisGlider)
{
	if (thisGlider->facing == kFaceLeft)
	{
		if (thisGlider->tipped)
			thisGlider->hVel += kHyperThrust;
		else
			thisGlider->hVel -= kHyperThrust;
	}
	else
	{
		if (thisGlider->tipped)
			thisGlider->hVel -= kHyperThrust;
		else
			thisGlider->hVel += kHyperThrust;
	}
	
	batteryTotal--;
	
	if (batteryTotal == 0)
	{
		QuickBatteryRefresh(false);
		PlayPrioritySound(kFizzleSound, kFizzlePriority);
	}
	else
	{
		if (!batteryWasEngaged)
			batteryFrame = 0;
		if (batteryFrame == 0)
			PlayPrioritySound(kThrustSound, kThrustPriority);
		batteryFrame++;
		if (batteryFrame >= 4)
			batteryFrame = 0;
		batteryWasEngaged = true;
	}
}

//--------------------------------------------------------------  DoHeliumEngaged

void DoHeliumEngaged (gliderPtr thisGlider)
{
	thisGlider->vDesiredVel = -kHeliumLift;
	batteryTotal++;
	
	if (batteryTotal == 0)
	{
		QuickBatteryRefresh(false);
		PlayPrioritySound(kFizzleSound, kFizzlePriority);
		batteryWasEngaged = false;
	}
	else
	{
		if (!batteryWasEngaged)
			batteryFrame = 0;
		if (batteryFrame == 0)
			PlayPrioritySound(kHissSound, kHissPriority);
		batteryFrame++;
		if (batteryFrame >= 4)
			batteryFrame = 0;
		batteryWasEngaged = true;
	}
}

//--------------------------------------------------------------  GetDemoInput

 void GetDemoInput (gliderPtr thisGlider)
 {
	const KeyDownStates *theKeys = PortabilityLayer::InputManager::GetInstance()->GetKeys();

 	if (thisGlider->which == kPlayer1)
	{
		
#if BUILD_ARCADE_VERSION
		
		if ((theKeys->IsSet(thisGlider->leftKey)) ||
			(theKeys->IsSet(thisGlider->gamepadLeftKey)) ||
				(theKeys->IsSet(thisGlider->rightKey)) ||
				(theKeys->IsSet(thisGlider->gamepadRightKey)) ||
				(theKeys->IsSet(thisGlider->battKey)) ||
				(theKeys->IsSet(thisGlider->gamepadBattKey)) ||
				(theKeys->IsSet(thisGlider->bandKey)) ||
				(theKeys->IsSet(thisGlider->gamepadBandKey)))
		{
			playing = false;
			paused = false;
		}
		
#else
		
		if (theKeys->IsSet(PL_KEY_EITHER_SPECIAL(kControl)))
			DoCommandKey();
		
#endif
	}
	
	if (thisGlider->mode == kGliderBurning)
	{
		if (thisGlider->facing == kFaceLeft)
			thisGlider->hDesiredVel -= kNormalThrust;
		else
			thisGlider->hDesiredVel += kNormalThrust;
	}
	else
	{
		thisGlider->heldLeft = false;
		thisGlider->heldRight = false;
		thisGlider->tipped = false;
		
	 	if (gameFrame == (long)demoData[demoIndex].frame)
	 	{
	 		switch (demoData[demoIndex].key)
	 		{
	 			case 0:		// left key
	 			thisGlider->hDesiredVel += kNormalThrust;
				thisGlider->tipped = (thisGlider->facing == kFaceLeft);
				thisGlider->heldRight = true;
				thisGlider->fireHeld = false;
	 			break;
	 			
	 			case 1:		// right key
	 			thisGlider->hDesiredVel -= kNormalThrust;
				thisGlider->tipped = (thisGlider->facing == kFaceRight);
				thisGlider->heldLeft = true;
				thisGlider->fireHeld = false;
	 			break;
	 			
	 			case 2:		// battery key
		 		if (batteryTotal > 0)
					DoBatteryEngaged(thisGlider);
				else
					DoHeliumEngaged(thisGlider);
	 			thisGlider->fireHeld = false;
	 			break;
	 			
	 			case 3:		// rubber band key
	 			if (!thisGlider->fireHeld)
				{
					if (AddBand(thisGlider, thisGlider->dest.left + 24, 
							thisGlider->dest.top + 10, thisGlider->facing))
					{
						bandsTotal--;
						if (bandsTotal <= 0)
							QuickBandsRefresh(false);
						
						thisGlider->fireHeld = true;
					}
				}
	 			break;
	 		}
	 		
	 		demoIndex++;
	 	}
	 	else
	 		thisGlider->fireHeld = false;
	 	
		if ((isEscPauseKey && theKeys->IsSet(PL_KEY_SPECIAL(kEscape))) ||
				(!isEscPauseKey && theKeys->IsSet(PL_KEY_SPECIAL(kTab))))
		{
			DoPause();
		}
 	}
 }
 
//--------------------------------------------------------------  GetInput

void GetInput (gliderPtr thisGlider)
{
	if (thisGlider->which == kPlayer1)
	{
		const KeyDownStates *theKeys = PortabilityLayer::InputManager::GetInstance()->GetKeys();

		if (theKeys->IsSet(PL_KEY_EITHER_SPECIAL(kControl)))
			DoCommandKey();
	}
	
	if (thisGlider->mode == kGliderBurning)
	{
		if (thisGlider->facing == kFaceLeft)
			thisGlider->hDesiredVel -= kNormalThrust;
		else
			thisGlider->hDesiredVel += kNormalThrust;
	}
	else
	{
		bool continuousFlipState = false;
		bool holdFlipState = false;
		bool leftState = false;
		bool rightState = false;

		const KeyDownStates *theKeys = PortabilityLayer::InputManager::GetInstance()->GetKeys();

		if (theKeys->IsSet(thisGlider->rightKey) || theKeys->IsSet(thisGlider->gamepadRightKey))			// right key
		{
			PL_NotYetImplemented_TODO("FixDemo");	// Flips aren't recorded in the demo properly

			if (theKeys->IsSet(thisGlider->leftKey) || theKeys->IsSet(thisGlider->gamepadLeftKey))
				continuousFlipState = true;
			else
				rightState = true;
		}
		else if (theKeys->IsSet(thisGlider->leftKey) || theKeys->IsSet(thisGlider->gamepadLeftKey))		// left key
			leftState = true;
		else
			thisGlider->tipped = false;

		if (theKeys->IsSet(thisGlider->gamepadRightKey))
			rightState = true;

		if (theKeys->IsSet(thisGlider->gamepadLeftKey))
			leftState = true;

		if (theKeys->IsSet(thisGlider->gamepadFaceLeftKey) && thisGlider->facing == kFaceRight)
			continuousFlipState = true;

		if (theKeys->IsSet(thisGlider->gamepadFaceRightKey) && thisGlider->facing == kFaceLeft)
			continuousFlipState = true;

		if (theKeys->IsSet(thisGlider->gamepadFlipKey))
			holdFlipState = true;

		if (thisGlider->which == kPlayer1 || thisGlider->which == kPlayer2)
		{
			unsigned int playerNum = 0;
			if (thisGlider->which == kPlayer1)
				playerNum = 0;
			else if (thisGlider->which == kPlayer2)
				playerNum = 1;

			int16_t inputAxis = PortabilityLayer::InputManager::GetInstance()->GetGamepadAxis(playerNum, GpGamepadAxes::kLeftStickX);
			if (inputAxis <= -kGamepadDeadzone)
				leftState = true;
			else if (inputAxis >= kGamepadDeadzone)
				rightState = true;
		}

		// gamepad flip key
		//if (BitTst(theKeys, thisGlider->gamepadFlipKey))
		//	holdFlipState = true;

		thisGlider->heldLeft = false;
		thisGlider->heldRight = false;
		if (continuousFlipState)
		{
			leftState = false;
			rightState = false;
			ToggleGliderFacing(thisGlider);
		}
		else if (holdFlipState)
		{
			if (!thisGlider->heldFlip)
			{
				ToggleGliderFacing(thisGlider);
				thisGlider->heldFlip = true;
			}
		}
		else
			thisGlider->heldFlip = false;

		if (rightState && !leftState)
		{
			thisGlider->hDesiredVel += kNormalThrust;
			thisGlider->tipped = (thisGlider->facing == kFaceLeft);
			thisGlider->heldRight = true;
		}

		if (leftState && !rightState)
		{
			thisGlider->hDesiredVel -= kNormalThrust;
			thisGlider->tipped = (thisGlider->facing == kFaceRight);
			thisGlider->heldLeft = true;
		}

		if (!leftState && !rightState)
			thisGlider->tipped = false;

		if ((theKeys->IsSet(thisGlider->battKey) || theKeys->IsSet(thisGlider->gamepadBattKey)) && (batteryTotal != 0) &&
				(thisGlider->mode == kGliderNormal))
		{
		#ifdef CREATEDEMODATA
			LogDemoKey(2);
		#endif
			if (batteryTotal > 0)
				DoBatteryEngaged(thisGlider);
			else
				DoHeliumEngaged(thisGlider);
		}
		else
			batteryWasEngaged = false;
		
		if ((theKeys->IsSet(thisGlider->bandKey) || theKeys->IsSet(thisGlider->gamepadBandKey)) && (bandsTotal > 0) &&
				(thisGlider->mode == kGliderNormal))
		{
		#ifdef CREATEDEMODATA
			LogDemoKey(3);
		#endif
			if (!thisGlider->fireHeld)
			{
				if (AddBand(thisGlider, thisGlider->dest.left + 24, 
						thisGlider->dest.top + 10, thisGlider->facing))
				{
					bandsTotal--;
					if (bandsTotal <= 0)
						QuickBandsRefresh(false);
					
					thisGlider->fireHeld = true;
				}
			}
		}
		else
			thisGlider->fireHeld = false;
		
		if ((otherPlayerEscaped != kNoOneEscaped) && 
				(theKeys->IsSet(PL_KEY_SPECIAL(kDelete))) &&
				(thisGlider->which) && (!onePlayerLeft))
		{
			ForceKillGlider();
		}
		
		if ((isEscPauseKey && theKeys->IsSet(PL_KEY_SPECIAL(kEscape))) ||
				(!isEscPauseKey && theKeys->IsSet(PL_KEY_SPECIAL(kTab))))
		{
			DoPause();
		}
	}
}

//--------------------------------------------------------------  QuerySaveGame

Boolean QuerySaveGame (void)
{
	#define		kSaveGameAlert		1041
	#define		kYesSaveGameButton	1
	short		hitWhat;
	
	InitCursor();
	FlushEvents();
//	CenterAlert(kSaveGameAlert);
	hitWhat = PortabilityLayer::DialogManager::GetInstance()->DisplayAlert(kSaveGameAlert, nullptr);
	if (hitWhat == kYesSaveGameButton)
		return (true);
	else
		return (false);
}

