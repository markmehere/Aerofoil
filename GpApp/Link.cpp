
//============================================================================
//----------------------------------------------------------------------------
//									  Link.c
//----------------------------------------------------------------------------
//============================================================================

#include "PLControlDefinitions.h"
#include "PLPasStr.h"
#include "Externs.h"
#include "Environ.h"
#include "ObjectEdit.h"
#include "RectUtils.h"
#include "PLButtonWidget.h"
#include "PLWidgets.h"
#include "WindowDef.h"
#include "WindowManager.h"

#define kLinkControlID			130
#define kUnlinkControlID		131


void DoLink (void);
void DoUnlink (void);


Rect			linkWindowRect;
PortabilityLayer::Widget	*linkControl, *unlinkControl;
WindowPtr		linkWindow;
short			isLinkH, isLinkV, linkRoom, linkType;
Byte			linkObject;
Boolean			isLinkOpen, linkerIsSwitch;


//==============================================================  Functions
//--------------------------------------------------------------  MergeFloorSuite

short MergeFloorSuite (short floor, short suite)
{
	return ((suite * 100) + floor);
}

//--------------------------------------------------------------  ExtractFloorSuite

void ExtractFloorSuite (short combo, short *floor, short *suite)
{
	if ((*thisHouse)->version < 0x0200)		// old floor/suite combo
	{
		*floor = (combo / 100) - kNumUndergroundFloors;
		*suite = combo % 100;
	}
	else
	{
		*suite = combo / 100;
		*floor = (combo % 100) - kNumUndergroundFloors;
	}
}

//--------------------------------------------------------------  UpdateLinkControl

void UpdateLinkControl (void)
{
#ifndef COMPILEDEMO
	if (linkWindow == nil)
		return;
	
	switch (linkType)
	{
	case kSwitchLinkOnly:
		if (objActive == kNoObjectSelected)
			linkControl->SetEnabled(false);// HiliteControl(linkControl, kControlInactive);
		else
			switch (thisRoom->objects[objActive].what)
			{
				case kFloorVent:
				case kCeilingVent:
				case kFloorBlower:
				case kCeilingBlower:
				case kSewerGrate:
				case kLeftFan:
				case kRightFan:
				case kInvisBlower:
				case kGrecoVent:
				case kSewerBlower:
				case kLiftArea:
				case kRedClock:
				case kBlueClock:
				case kYellowClock:
				case kCuckoo:
				case kPaper:
				case kBattery:
				case kBands:
				case kFoil:
				case kInvisBonus:
				case kHelium:
				case kDeluxeTrans:
				case kCeilingLight:
				case kLightBulb:
				case kTableLamp:
				case kHipLamp:
				case kDecoLamp:
				case kFlourescent:
				case kTrackLight:
				case kInvisLight:
				case kShredder:
				case kToaster:
				case kMacPlus:
				case kTV:
				case kCoffee:
				case kOutlet:
				case kVCR:
				case kStereo:
				case kMicrowave:
				case kBalloon:
				case kCopterLf:
				case kCopterRt:
				case kDartLf:
				case kDartRt:
				case kBall:
				case kDrip:
				case kFish:
					linkControl->SetEnabled(true);
				break;
				
				default:
					linkControl->SetEnabled(false);
				break;
			}
		break;
		
		case kTriggerLinkOnly:
		if (objActive == kNoObjectSelected)
			linkControl->SetEnabled(false);
		else
			switch (thisRoom->objects[objActive].what)
			{
				case kGreaseRt:
				case kGreaseLf:
				case kToaster:
				case kGuitar:
				case kCoffee:
				case kOutlet:
				case kBalloon:
				case kCopterLf:
				case kCopterRt:
				case kDartLf:
				case kDartRt:
				case kDrip:
				case kFish:
					linkControl->SetEnabled(true);
				break;
				
				case kLightSwitch:
				case kMachineSwitch:
				case kThermostat:
				case kPowerSwitch:
				case kKnifeSwitch:
				case kInvisSwitch:
				if (linkRoom == thisRoomNumber)
					linkControl->SetEnabled(true);
				break;
				
				default:
					linkControl->SetEnabled(false);
				break;
			}
		break;
		
		case kTransportLinkOnly:
		if (objActive == kNoObjectSelected)
			linkControl->SetEnabled(false);
		else
			switch (thisRoom->objects[objActive].what)
			{
				case kMailboxLf:
				case kMailboxRt:
				case kCeilingTrans:
				case kInvisTrans:
				case kDeluxeTrans:
				case kInvisLight:
				case kOzma:
				case kMirror:
				case kFireplace:
				case kWallWindow:
				case kCalendar:
				case kBulletin:
				case kCloud:
					linkControl->SetEnabled(true);
				break;
				
				default:
					linkControl->SetEnabled(false);
				break;
			}
		break;
	}
#endif
}

//--------------------------------------------------------------  UpdateLinkWindow

void UpdateLinkWindow (void)
{
#ifndef COMPILEDEMO
	if (linkWindow == nil)
		return;
	
	SetPortWindowPort(linkWindow);
	UpdateLinkControl();
#endif
}

//--------------------------------------------------------------  OpenLinkWindow

void OpenLinkWindow (void)
{
#ifndef COMPILEDEMO
	Rect		src, dest;
	Point		globalMouse;

	PortabilityLayer::WindowManager *wm = PortabilityLayer::WindowManager::GetInstance();
	
	if (linkWindow == nil)
	{
		const uint16_t windowStyle = PortabilityLayer::WindowStyleFlags::kTitleBar | PortabilityLayer::WindowStyleFlags::kMiniBar | PortabilityLayer::WindowStyleFlags::kCloseBox;

		QSetRect(&linkWindowRect, 0, 0, 129, 30);

		{
			PortabilityLayer::WindowDef wdef = PortabilityLayer::WindowDef::Create(linkWindowRect, windowStyle, true, 0, 0, PSTR("Link"));
			linkWindow = wm->CreateWindow(wdef);
		}
		
		wm->PutWindowBehind(linkWindow, wm->GetPutInFrontSentinel());

		MoveWindow(linkWindow, isLinkH, isLinkV, true);

		GetWindowRect(linkWindow, &dest);
		PortabilityLayer::WindowManager::GetInstance()->ShowWindow(linkWindow);
//		FlagWindowFloating(linkWindow);	TEMP - use flaoting windows

		PortabilityLayer::WidgetBasicState basicState;
		basicState.m_rect = Rect::Create(5, 70, 25, 124);
		basicState.m_text.Set(4, "Link");
		basicState.m_window = linkWindow;

		linkControl = PortabilityLayer::ButtonWidget::Create(basicState, nullptr);

		basicState.m_rect = Rect::Create(5, 5, 25, 59);
		basicState.m_text.Set(6, "Unlink");
		basicState.m_window = linkWindow;
		unlinkControl = PortabilityLayer::ButtonWidget::Create(basicState, nullptr);

		linkWindow->DrawControls();
		
		linkRoom = -1;
		linkObject = 255;
		
		isLinkOpen = true;

		UpdateLinkWindow();
	}
#endif
}

//--------------------------------------------------------------  CloseLinkWindow

void CloseLinkWindow (void)
{
#ifndef COMPILEDEMO
	if (linkWindow != nil)
		PortabilityLayer::WindowManager::GetInstance()->DestroyWindow(linkWindow);
	
	linkWindow = nil;
	isLinkOpen = false;
#endif
}

//--------------------------------------------------------------  DoLink

#ifndef COMPILEDEMO
void DoLink (void)
{
	short		floor, suite;
	char		wasState;
	
	if (GetRoomFloorSuite(thisRoomNumber, &floor, &suite))
	{
		floor += kNumUndergroundFloors;
		if (thisRoomNumber == linkRoom)
		{
			if (linkerIsSwitch)
			{
				thisRoom->objects[linkObject].data.e.where = 
						MergeFloorSuite(floor, suite);
				thisRoom->objects[linkObject].data.e.who = 
						objActive;
			}
			else
			{
				thisRoom->objects[linkObject].data.d.where = 
						MergeFloorSuite(floor, suite);
				thisRoom->objects[linkObject].data.d.who = 
						objActive;
			}
		}
		else
		{
			if (linkerIsSwitch)
			{
				(*thisHouse)->rooms[linkRoom].objects[linkObject].data.e.where = 
						MergeFloorSuite(floor, suite);
				(*thisHouse)->rooms[linkRoom].objects[linkObject].data.e.who = 
						objActive;
			}
			else	// linker is transport
			{
				(*thisHouse)->rooms[linkRoom].objects[linkObject].data.d.where = 
						MergeFloorSuite(floor, suite);
				(*thisHouse)->rooms[linkRoom].objects[linkObject].data.d.who = 
						objActive;
			}
		}
		fileDirty = true;
		UpdateMenus(false);
		CloseLinkWindow();
	}
}
#endif

//--------------------------------------------------------------  DoUnlink

#ifndef COMPILEDEMO
void DoUnlink (void)
{
	char		wasState;
	
	if (thisRoomNumber == linkRoom)
	{
		if (linkerIsSwitch)
		{
			thisRoom->objects[linkObject].data.e.where = -1;
			thisRoom->objects[linkObject].data.e.who = 255;
		}
		else
		{
			thisRoom->objects[linkObject].data.d.where = -1;
			thisRoom->objects[linkObject].data.d.who = 255;
		}
	}
	else
	{
		if (linkerIsSwitch)
		{
			(*thisHouse)->rooms[linkRoom].objects[linkObject].data.e.where = -1;
			(*thisHouse)->rooms[linkRoom].objects[linkObject].data.e.who = 255;
		}
		else
		{
			(*thisHouse)->rooms[linkRoom].objects[linkObject].data.d.where = -1;
			(*thisHouse)->rooms[linkRoom].objects[linkObject].data.d.who = 255;
		}
	}
	fileDirty = true;
	UpdateMenus(false);
	CloseLinkWindow();
}
#endif

//--------------------------------------------------------------  HandleLinkClick

void HandleLinkClick (Point wherePt)
{
#ifndef COMPILEDEMO
	PortabilityLayer::Widget	*theControl;
	short			part;
	
	if (linkWindow == nil)
		return;
	
	SetPortWindowPort(linkWindow);
	wherePt -= linkWindow->GetTopLeftCoord();
	
	part = FindControl(wherePt, linkWindow, &theControl);
	if ((theControl != nil) && (part != 0))
	{
		part = theControl->Capture(wherePt, nullptr);
		if (part != 0)
		{
			if (theControl == linkControl)
				DoLink();
			else if (theControl == unlinkControl)
				DoUnlink();
			
			if (thisRoomNumber == linkRoom)
				CopyThisRoomToRoom();
			GenerateRetroLinks();
		}
	}
#endif
}

