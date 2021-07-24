//============================================================================
//----------------------------------------------------------------------------
//									Input.c
//----------------------------------------------------------------------------
//============================================================================


#include "PLDialogs.h"
#include "PLKeyEncoding.h"
#include "BitmapImage.h"
#include "DialogManager.h"
#include "Environ.h"
#include "Externs.h"
#include "FontFamily.h"
#include "InputManager.h"
#include "MainWindow.h"
#include "PLStandardColors.h"
#include "PLTimeTaggedVOSEvent.h"
#include "QDPixMap.h"
#include "RectUtils.h"
#include "RenderedFont.h"
#include "GpRenderedFontMetrics.h"
#include "ResolveCachingColor.h"
#include "ResourceManager.h"
#include "Utilities.h"
#include "Vec2i.h"
#include "WindowDef.h"
#include "WindowManager.h"


#define kNormalThrust		5
#define kHyperThrust		8
#define kHeliumLift			4
#define kEscPausePictID		1015
#define kTabPausePictID		1016
#define	kSavingGameDial		1042


void LogDemoKey (char);
void DoCommandKey (void);
void DoPause (void);
void DoTouchScreenMenu (void);
void DoBatteryEngaged (gliderPtr);
void DoHeliumEngaged (gliderPtr);
void DoEndGame (void);
void QuerySaveGame (Boolean &save, Boolean &cancel);


demoPtr		demoData;
Dialog		*saveDial;
short		demoIndex, batteryFrame;
Boolean		isEscPauseKey, paused, batteryWasEngaged;

extern	long		gameFrame;
extern	short		otherPlayerEscaped;
extern	Boolean		quitting, playing, onePlayerLeft, twoPlayerGame, demoGoing, pendingTouchScreenMenu;
extern	touchScreenControlState	touchScreen;
extern	macEnviron	thisMac;


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
		DoEndGame();
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

//--------------------------------------------------------------  DrawTouchScreenMenu

namespace TouchScreenMenuMetrics
{
	const int kTextLineSpacing = 70;
	const int kTextLeftX = 36;
	const int kTextFirstY = 48;
	const int kTextSize = 18;

	const int kHighlightXOffset = 10;
	const int kHighlightYOffset = 58;
}

namespace TouchScreenMenuItems
{
	enum TouchScreenMenuItem
	{
		kResume,
		kSave,
		kQuit,

		kCount,
	};
}

typedef TouchScreenMenuItems::TouchScreenMenuItem TouchScreenMenuItem_t;



//--------------------------------------------------------------  IsTouchScreenMenuItemEnabled

Boolean IsTouchScreenMenuItemEnabled(int index)
{
	if (index == TouchScreenMenuItems::kSave)
		return !twoPlayerGame;

	return true;
}

//--------------------------------------------------------------  DrawTouchScreenMenu

void DrawTouchScreenMenu (DrawSurface *surface, const THandle<BitmapImage> &backgroundImage, const DrawSurface *highlightSurface, int selectedItemIndex)
{
	surface->DrawPicture(backgroundImage, (*backgroundImage)->GetRect());

	const PLPasStr itemTexts[TouchScreenMenuItems::kCount] =
	{
		PSTR("Resume"),
		PSTR("Save"),
		PSTR("Quit"),
	};

	const int numItems = sizeof(itemTexts) / sizeof(itemTexts[0]);

	PortabilityLayer::RenderedFont *rfont = GetFont(PortabilityLayer::FontPresets::kHandwriting48);
	if (!rfont)
		return;

	const int32_t fontAscent = rfont->GetMetrics().m_ascent;
	const int32_t firstY = TouchScreenMenuMetrics::kTextFirstY + (fontAscent + TouchScreenMenuMetrics::kTextLineSpacing + 1) / 2;

	PortabilityLayer::ResolveCachingColor blackColor(StdColors::Black());
	PortabilityLayer::ResolveCachingColor grayColor(PortabilityLayer::RGBAColor::Create(120, 120, 120, 255));

	for (int i = 0; i < numItems; i++)
	{
		PortabilityLayer::ResolveCachingColor &selectedColor = IsTouchScreenMenuItemEnabled(i) ? blackColor : grayColor;

		surface->DrawString(Point::Create(TouchScreenMenuMetrics::kTextLeftX, firstY + i * TouchScreenMenuMetrics::kTextLineSpacing), itemTexts[i], selectedColor, rfont);
	}

	if (selectedItemIndex >= 0)
	{
		BitMap *highlightBitmap = *highlightSurface->m_port.GetPixMap();
		Rect highlightRect = highlightBitmap->m_rect;
		Rect highlightDestRect = highlightRect;
		highlightDestRect += Point::Create(TouchScreenMenuMetrics::kHighlightXOffset, TouchScreenMenuMetrics::kHighlightYOffset + selectedItemIndex * TouchScreenMenuMetrics::kTextLineSpacing);

		CopyMask(highlightBitmap, highlightBitmap, *surface->m_port.GetPixMap(), &highlightRect, &highlightRect, &highlightDestRect);
	}
}

//--------------------------------------------------------------  DoTouchScreenMenu

void DoTouchScreenMenu(void)
{
	static const int kTouchScreenMenuResource = 1300;
	static const int kTouchScreenHighlightResource = 1301;

	THandle<BitmapImage> highlightH = PortabilityLayer::ResourceManager::GetInstance()->GetAppResource('PICT', kTouchScreenHighlightResource).StaticCast<BitmapImage>();
	BitmapImage *highlightImage = *highlightH;

	if (!highlightH)
		return;

	DrawSurface *highlightSurface = nullptr;
	Rect highlightRect = highlightImage->GetRect();
	if (CreateOffScreenGWorld(&highlightSurface, &highlightRect) != PLErrors::kNone)
	{
		highlightH.Dispose();
		return;
	}

	highlightSurface->DrawPicture(highlightH, highlightRect);
	highlightH.Dispose();

	BitMap *highlightBitmap = *highlightSurface->m_port.GetPixMap();

	THandle<BitmapImage> imageH = PortabilityLayer::ResourceManager::GetInstance()->GetAppResource('PICT', kTouchScreenMenuResource).StaticCast<BitmapImage>();

	if (!imageH)
		return;

	BitmapImage *image = *imageH;
	Rect menuRect = image->GetRect();

	const uint16_t width = image->GetRect().Width();
	const uint16_t height = image->GetRect().Height();

	uint16_t wx = (thisMac.fullScreen.left + thisMac.fullScreen.right - width) / 2;
	uint16_t wy = (thisMac.fullScreen.top + thisMac.fullScreen.bottom - height) / 2;

	PortabilityLayer::WindowDef wdef = PortabilityLayer::WindowDef::Create(menuRect, PortabilityLayer::WindowStyleFlags::kBorderless, true, 0, 0, PSTR(""));
	PortabilityLayer::WindowManager *wm = PortabilityLayer::WindowManager::GetInstance();

	Window *window = wm->CreateWindow(wdef);
	wm->PutWindowBehind(window, wm->GetPutInFrontSentinel());
	window->SetPosition(PortabilityLayer::Vec2i(wx, wy));

	Window *exclWindow = window;
	wm->SwapExclusiveWindow(exclWindow);

	DrawSurface *surface = window->GetDrawSurface();

	DrawTouchScreenMenu(surface, imageH, highlightSurface, -1);

	PortabilityLayer::ResolveCachingColor blackColor(StdColors::Black());
	PortabilityLayer::ResolveCachingColor grayColor(StdColors::Black());

	int lockedItem = -1;
	int highlightedItem = -1;
	for (;;)
	{
		TimeTaggedVOSEvent evt;
		if (WaitForEvent(&evt, 1))
		{
			int newLockedItem = lockedItem;
			int newHighlightedItem = highlightedItem;
			if (evt.IsLMouseDownEvent())
			{
				const Point mousePt = window->MouseToLocal(evt.m_vosEvent.m_event.m_mouseInputEvent);
				if (!menuRect.Contains(mousePt))
					continue;

				newLockedItem = -1;
				if (mousePt.v >= TouchScreenMenuMetrics::kTextFirstY)
				{
					int itemV = mousePt.v - TouchScreenMenuMetrics::kTextFirstY;
					int itemIndex = itemV / TouchScreenMenuMetrics::kTextLineSpacing;

					if (itemIndex >= 0 && itemIndex < TouchScreenMenuItems::kCount)
					{
						newLockedItem = itemIndex;
						newHighlightedItem = itemIndex;
					}
				}
			}
			else if (evt.m_vosEvent.m_eventType == GpVOSEventTypes::kMouseInput)
			{
				const GpMouseInputEvent &mouseEvt = evt.m_vosEvent.m_event.m_mouseInputEvent;
				if (mouseEvt.m_eventType == GpMouseEventTypes::kLeave)
				{
					newHighlightedItem = -1;
					newLockedItem = -1;
				}
				else if (mouseEvt.m_eventType == GpMouseEventTypes::kMove || mouseEvt.m_eventType == GpMouseEventTypes::kUp)
				{
					const Point mousePt = window->MouseToLocal(mouseEvt);
					newHighlightedItem = -1;

					if (menuRect.Contains(mousePt) && mousePt.v >= TouchScreenMenuMetrics::kTextFirstY)
					{
						int itemV = mousePt.v - TouchScreenMenuMetrics::kTextFirstY;
						int itemIndex = itemV / TouchScreenMenuMetrics::kTextLineSpacing;

						if (itemIndex >= 0 && itemIndex < TouchScreenMenuItems::kCount)
						{
							if (itemIndex == lockedItem)
								newHighlightedItem = itemIndex;
							else
								newHighlightedItem = -1;
						}
					}

					if (mouseEvt.m_eventType == GpMouseEventTypes::kUp)
					{
						if (newHighlightedItem >= 0)
						{
							highlightedItem = newHighlightedItem;
							break;
						}
						else
						{
							newLockedItem = -1;
							newHighlightedItem = -1;
						}
					}
				}
			}

			if (newLockedItem != lockedItem)
				lockedItem = newLockedItem;

			if (newHighlightedItem != highlightedItem)
			{
				highlightedItem = newHighlightedItem;
				DrawTouchScreenMenu(surface, imageH, highlightSurface, highlightedItem);
			}
		}
	}

	wm->SwapExclusiveWindow(exclWindow);

	wm->DestroyWindow(window);
	imageH.Dispose();
	DisposeGWorld(highlightSurface);

	if (highlightedItem < 0)
		return;

	switch (highlightedItem)
	{
	case TouchScreenMenuItems::kQuit:
		DoEndGame();
		break;
	case TouchScreenMenuItems::kSave:
		assert(!twoPlayerGame);
		RefreshScoreboard(kSavingTitleMode);
		SaveGame2();				// New save game.
		HideCursor();
		CopyRectWorkToMain(&workSrcRect);
		RefreshScoreboard(kNormalTitleMode);
		break;
	default:
		break;
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
		LoadScaledGraphicCustom(surface, kEscPausePictID, &bounds);
	else
		LoadScaledGraphicCustom(surface, kTabPausePictID, &bounds);

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

		// Cheesy - Use touchscreen menu as quit
		if (pendingTouchScreenMenu)
		{
			playing = false;
			paused = false;
		}
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

	if (pendingTouchScreenMenu)
	{
		pendingTouchScreenMenu = false;
		DoTouchScreenMenu();
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
		bool bandsState = false;
		bool batteryState = false;

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

		if (thisMac.isTouchscreen)
		{
			bool touchLeftState = false;
			bool touchRightState = false;

			for (int fi = 0; fi < touchScreenControlState::kMaxFingers; fi++)
			{
				const touchScreenFingerState &fstate = touchScreen.fingers[fi];
				if (!fstate.active)
					continue;

				const Point touchScreenPoint = touchScreen.fingers[fi].point;

				if (!touchScreen.controls[fstate.capturingControl].touchRect.Contains(touchScreenPoint))
					continue;

				switch (fstate.capturingControl)
				{
				case TouchScreenCtrlIDs::Movement:
					{
						int32_t screenWidth = mainWindowRect.Width();
						if (touchScreenPoint.h * 2 <= screenWidth)
							touchLeftState = true;
						if (touchScreenPoint.h * 2 - screenWidth >= 0)
							touchRightState = true;
					}
					break;
				case TouchScreenCtrlIDs::Bands:
					bandsState = true;
					break;
				case TouchScreenCtrlIDs::BatteryHelium:
					batteryState = true;
					break;
				default:
					break;
				}
			}

			if (touchLeftState)
			{
				if (touchRightState)
					continuousFlipState = true;
				else
					leftState = true;
			}
			else if (touchRightState)
				rightState = true;
		}

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

		if ((theKeys->IsSet(thisGlider->battKey) || theKeys->IsSet(thisGlider->gamepadBattKey) || batteryState) && (batteryTotal != 0) &&
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

		if ((theKeys->IsSet(thisGlider->bandKey) || theKeys->IsSet(thisGlider->gamepadBandKey) || bandsState) && (bandsTotal > 0) &&
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

void DoEndGame() {
	Boolean wantCancel = false;
	playing = false;
	paused = false;
	quitting = false;
	if ((!twoPlayerGame) && (!demoGoing))
	{
		Boolean wantSave = false;
		QuerySaveGame(wantSave, wantCancel);
		if (wantSave)
		{
			if (!SaveGame2())        // New save game.
				wantCancel = true;
		}
	}
	
	if (wantCancel)
	{
		playing = true;
	}
}

//--------------------------------------------------------------  QuerySaveGame

void QuerySaveGame (Boolean &save, Boolean &cancel)
{
	#define		kSaveGameAlert		1041
	#define		kYesSaveGameButton	1
	#define		kNoSaveGameButton			2
	#define		kCancelSaveGameButton		3
	short		hitWhat;

	InitCursor();
	FlushEvents();
//	CenterAlert(kSaveGameAlert);
	hitWhat = PortabilityLayer::DialogManager::GetInstance()->DisplayAlert(kSaveGameAlert, nullptr);
	save = (hitWhat == kYesSaveGameButton);
	cancel = (hitWhat == kCancelSaveGameButton);
}

