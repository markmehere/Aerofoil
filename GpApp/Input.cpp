//============================================================================
//----------------------------------------------------------------------------
//									Input.c
//----------------------------------------------------------------------------
//============================================================================


#include "PLToolUtils.h"
#include "PLDialogs.h"
#include "PLKeyEncoding.h"
#include "Externs.h"
#include "InputManager.h"
#include "MainWindow.h"
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
KeyMap		theKeys;
DialogPtr	saveDial;
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
	if (BitTst(theKeys, PL_KEY_ASCII('Q')))
	{
		playing = false;
		paused = false;
		if ((!twoPlayerGame) && (!demoGoing))
		{
			if (QuerySaveGame())
				SaveGame2();		// New save game.
		}
	}
	else if ((BitTst(theKeys, PL_KEY_ASCII('S'))) && (!twoPlayerGame))
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
	
	do
	{
		GetKeys(theKeys);
		Delay(1, nullptr);
	}
	while ((isEscPauseKey && BitTst(theKeys, PL_KEY_SPECIAL(kEscape))) || 
			(!isEscPauseKey && BitTst(theKeys, PL_KEY_SPECIAL(kTab))));
	
	paused = true;
	while (paused)
	{
		GetKeys(theKeys);
		if ((isEscPauseKey && BitTst(theKeys, PL_KEY_SPECIAL(kEscape))) ||
				(!isEscPauseKey && BitTst(theKeys, PL_KEY_SPECIAL(kTab))))
			paused = false;
		else if (BitTst(theKeys, PL_KEY_EITHER_SPECIAL(kControl)))
			DoCommandKey();

		Delay(1, nullptr);
	}
	
	CopyBits((BitMap *)*GetGWorldPixMap(workSrcMap), 
			GetPortBitMapForCopyBits(GetWindowPort(mainWindow)), 
			&bounds, &bounds, srcCopy);
	
	do
	{
		GetKeys(theKeys);
		Delay(1, nullptr);
	}
	while ((isEscPauseKey && BitTst(theKeys, PL_KEY_SPECIAL(kEscape))) ||
			(!isEscPauseKey && BitTst(theKeys, PL_KEY_SPECIAL(kTab))));
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
 	if (thisGlider->which == kPlayer1)
	{
		GetKeys(theKeys);
		
#if BUILD_ARCADE_VERSION
		
		if ((BitTst(theKeys, thisGlider->leftKey)) ||
			(BitTst(theKeys, thisGlider->gamepadLeftKey)) ||
				(BitTst(theKeys, thisGlider->rightKey)) ||
				(BitTst(theKeys, thisGlider->gamepadRightKey)) ||
				(BitTst(theKeys, thisGlider->battKey)) ||
				(BitTst(theKeys, thisGlider->gamepadBattKey)) ||
				(BitTst(theKeys, thisGlider->bandKey)) ||
				(BitTst(theKeys, thisGlider->gamepadBandKey)))
		{
			playing = false;
			paused = false;
		}
		
#else
		
		if (BitTst(&theKeys, kCommandKeyMap))
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
	 	
		if ((isEscPauseKey && BitTst(theKeys, PL_KEY_SPECIAL(kEscape))) ||
				(!isEscPauseKey && BitTst(theKeys, PL_KEY_SPECIAL(kTab))))
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
		GetKeys(theKeys);
		if (BitTst(theKeys, PL_KEY_EITHER_SPECIAL(kControl)))
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

		if (BitTst(theKeys, thisGlider->rightKey) || BitTst(theKeys, thisGlider->gamepadRightKey))			// right key
		{
			PL_NotYetImplemented_TODO("FixDemo");	// Flips aren't recorded in the demo properly

			if (BitTst(theKeys, thisGlider->leftKey) || BitTst(theKeys, thisGlider->gamepadLeftKey))
				continuousFlipState = true;
			else
				rightState = true;
		}
		else if (BitTst(theKeys, thisGlider->leftKey) || BitTst(theKeys, thisGlider->gamepadLeftKey))		// left key
			leftState = true;
		else
			thisGlider->tipped = false;

		if (BitTst(theKeys, thisGlider->gamepadRightKey))
			rightState = true;

		if (BitTst(theKeys, thisGlider->gamepadLeftKey))
			leftState = true;

		if (BitTst(theKeys, thisGlider->gamepadFaceLeftKey) && thisGlider->facing == kFaceRight)
			continuousFlipState = true;

		if (BitTst(theKeys, thisGlider->gamepadFaceRightKey) && thisGlider->facing == kFaceLeft)
			continuousFlipState = true;

		if (BitTst(theKeys, thisGlider->gamepadFlipKey))
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

		if ((BitTst(theKeys, thisGlider->battKey) || BitTst(theKeys, thisGlider->gamepadBattKey)) && (batteryTotal != 0) && 
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
		
		if ((BitTst(theKeys, thisGlider->bandKey) || BitTst(theKeys, thisGlider->gamepadBandKey)) && (bandsTotal > 0) &&
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
				(BitTst(theKeys, PL_KEY_SPECIAL(kDelete))) &&
				(thisGlider->which) && (!onePlayerLeft))
		{
			ForceKillGlider();
		}
		
		if ((isEscPauseKey && BitTst(theKeys, PL_KEY_SPECIAL(kEscape))) ||
				(!isEscPauseKey && BitTst(theKeys, PL_KEY_SPECIAL(kTab))))
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
	FlushEvents(everyEvent, 0);
//	CenterAlert(kSaveGameAlert);
	hitWhat = Alert(kSaveGameAlert, nil);
	if (hitWhat == kYesSaveGameButton)
		return (true);
	else
		return (false);
}

