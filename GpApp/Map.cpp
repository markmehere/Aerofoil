//============================================================================
//----------------------------------------------------------------------------
//									 Map.c
//----------------------------------------------------------------------------
//============================================================================


#include "PLControlDefinitions.h"
#include "PLResources.h"
#include "PLPasStr.h"
#include "PLStandardColors.h"
#include "Externs.h"
#include "Environ.h"
#include "House.h"
#include "PLScrollBarWidget.h"
#include "PLWidgets.h"
#include "WindowDef.h"
#include "WindowManager.h"
#include "RectUtils.h"
#include "Utilities.h"


#define kMapRoomsHigh			9	// was 7
#define kMapRoomsWide			9	// was 7
#define kMapScrollBarWidth		16
#define kHScrollRef				5L
#define kVScrollRef				27L
#define kMapGroundValue			56
#define kNewRoomAlert			1004
#define kYesDoNewRoom			1
#define kThumbnailPictID		1010


void LoadGraphicPlus (DrawSurface *, short, const Rect &);
void RedrawMapContents (void);
void LiveHScrollAction (ControlHandle, short);
void LiveVScrollAction (ControlHandle, short);
Boolean QueryNewRoom (void);
void CreateNailOffscreen (void);
void KillNailOffscreen (void);

Rect			nailSrcRect, activeRoomRect, wasActiveRoomRect;
Rect			mapHScrollRect, mapVScrollRect, mapCenterRect;
Rect			mapWindowRect;
DrawSurface		*nailSrcMap;
WindowPtr		mapWindow;
PortabilityLayer::Widget	*mapHScroll, *mapVScroll;
short			isMapH, isMapV, mapRoomsHigh, mapRoomsWide;
short			mapLeftRoom, mapTopRoom;
Boolean			isMapOpen, doPrettyMap;

extern	Boolean		doBitchDialogs;


//==============================================================  Functions
//--------------------------------------------------------------  ThisRoomVisibleOnMap

#ifndef COMPILEDEMO
Boolean ThisRoomVisibleOnMap (void)
{
	short		h, v;
	
	h = thisRoom->suite;
	v = kMapGroundValue - thisRoom->floor;
	
	if ((h < mapLeftRoom) || (v < mapTopRoom) || 
			(h >= (mapLeftRoom + mapRoomsWide)) || 
			(v >= (mapTopRoom + mapRoomsHigh)))
		return (false);
	else
		return (true);
}
#endif

//--------------------------------------------------------------  CenterMapOnRoom

#ifndef COMPILEDEMO
void CenterMapOnRoom (short h, short v)
{
	if (mapWindow == nil)
		return;
	
	mapLeftRoom = h - (mapRoomsWide / 2);
	mapTopRoom = (kMapGroundValue - v) - (mapRoomsHigh / 2);
	
	if (mapLeftRoom < 0)
		mapLeftRoom = 0;
	else if (mapLeftRoom > (kMaxNumRoomsH - mapRoomsWide))
		mapLeftRoom = kMaxNumRoomsH - mapRoomsWide;
	
	if (mapTopRoom < 0)
		mapTopRoom = 0;
	else if (mapTopRoom > (kMaxNumRoomsV - mapRoomsHigh))
		mapTopRoom = kMaxNumRoomsV - mapRoomsHigh;
	
	if (mapWindow != nil)
	{
		mapHScroll->SetState(mapLeftRoom);
		mapVScroll->SetState(mapTopRoom);
	}
}
#endif

//--------------------------------------------------------------  FlagMapRoomsForUpdate

#ifndef COMPILEDEMO
void FlagMapRoomsForUpdate (void)
{
	if (mapWindow == nil)
		return;
	
//	SetPortWindowPort(mapWindow);
	InvalWindowRect(mapWindow, &wasActiveRoomRect);
	InvalWindowRect(mapWindow, &activeRoomRect);
}
#endif

//--------------------------------------------------------------  FindNewActiveRoomRect

#ifndef COMPILEDEMO
void FindNewActiveRoomRect (void)
{
	Rect		aRoom;
	short		h, i;
	short		floor, suite, whoCares;
	char		wasState;
	Boolean		activeRoomVisible;
	
	if (mapWindow == nil)
		return;
	
	activeRoomVisible = false;
	
	for (i = 0; i < mapRoomsHigh; i++)
	{
		for (h = 0; h < mapRoomsWide; h++)
		{
			QSetRect(&aRoom, 0, 0, kMapRoomWidth, kMapRoomHeight);
			QOffsetRect(&aRoom, kMapRoomWidth * h, kMapRoomHeight * i);
			
			suite = h + mapLeftRoom;
			floor = kMapGroundValue - (i + mapTopRoom);
			if ((RoomExists(suite, floor, &whoCares)) && (houseUnlocked))
			{
				if (whoCares == thisRoomNumber)
				{
					wasActiveRoomRect = activeRoomRect;
					activeRoomRect = aRoom;
					activeRoomVisible = true;
				}
			}
		}
	}
	
	if (activeRoomVisible)
	{
		activeRoomRect.right++;
		activeRoomRect.bottom++;
		InsetRect(&activeRoomRect, -1, -1);
	}
}
#endif

//--------------------------------------------------------------  LoadGraphicPlus

void LoadGraphicPlus (DrawSurface *surface, short resID, const Rect &theRect)
{
	THandle<BitmapImage>	thePicture;
	
	thePicture = LoadHouseResource('PICT', resID).StaticCast<BitmapImage>();
	if (thePicture == nil)
	{
		thePicture = LoadHouseResource('Date', resID).StaticCast<BitmapImage>();
		if (thePicture == nil)
		{
			return;
		}
	}
	surface->DrawPicture(thePicture, theRect);
	thePicture.Dispose();
}

//--------------------------------------------------------------  RedrawMapContents

#ifndef COMPILEDEMO
void RedrawMapContents (void)
{
	Rect		newClip, aRoom, src;
	short		h, i, groundLevel;
	short		floor, suite, whoCares, type;
	char		wasState;
	Boolean		activeRoomVisible;
	
	if (mapWindow == nil)
		return;
	
	activeRoomVisible = false;
	groundLevel = kMapGroundValue - mapTopRoom;
	
	newClip.left = mapWindowRect.left;
	newClip.top = mapWindowRect.top;
	newClip.right = mapWindowRect.right + 2 - kMapScrollBarWidth;
	newClip.bottom = mapWindowRect.bottom + 2 - kMapScrollBarWidth;

	DrawSurface *surface = mapWindow->GetDrawSurface();

	const Rect wasClip = surface->GetClipRect();
	surface->SetClipRect(newClip);
	
	for (i = 0; i < mapRoomsHigh; i++)
	{
		for (h = 0; h < mapRoomsWide; h++)
		{
			QSetRect(&aRoom, 0, 0, kMapRoomWidth, kMapRoomHeight);
			QOffsetRect(&aRoom, kMapRoomWidth * h, kMapRoomHeight * i);
			
			suite = h + mapLeftRoom;
			floor = kMapGroundValue - (i + mapTopRoom);
			if ((RoomExists(suite, floor, &whoCares)) && (houseUnlocked))
			{
				type = (*thisHouse)->rooms[whoCares].background - kBaseBackgroundID;
				if (type > kNumBackgrounds)
				{
					if (!doPrettyMap)
						type = kNumBackgrounds;	// Draw "?" thumbnail.
				}

				surface->SetForeColor(StdColors::Black());
				if (type > kNumBackgrounds)		// Do a "pretty" thumbnail.
				{
					LoadGraphicPlus(surface, type + kBaseBackgroundID, aRoom);
				}
				else
				{
					QSetRect(&src, 0, 0, kMapRoomWidth, kMapRoomHeight);
					QOffsetRect(&src, 0, type * kMapRoomHeight);
					CopyBits((BitMap *)*GetGWorldPixMap(nailSrcMap), 
							GetPortBitMapForCopyBits(GetWindowPort(mapWindow)), 
							&src, &aRoom, srcCopy);
				}
				
				if (whoCares == thisRoomNumber)
				{
					activeRoomRect = aRoom;
					activeRoomVisible = true;
				}
			}
			else
			{
				if (i >= groundLevel)
					surface->SetForeColor(StdColors::Green());
				else
					surface->SetForeColor(StdColors::Blue());

				Pattern dummyPat;
				surface->FillRectWithPattern8x8(aRoom, *GetQDGlobalsGray(&dummyPat));
			}
		}
	}

	surface->SetForeColor(StdColors::Black());
	
	for (i = 1; i < mapRoomsWide; i++)
	{
		const Point upperPoint = Point::Create(i * kMapRoomWidth, 0);
		const Point lowerPoint = Point::Create(upperPoint.h, upperPoint.v + mapRoomsHigh * kMapRoomHeight);
		surface->DrawLine(upperPoint, lowerPoint);
	}
	
	for (i = 1; i < mapRoomsHigh; i++)
	{
		const Point leftPoint = Point::Create(0, i * kMapRoomHeight);
		const Point rightPoint = leftPoint + Point::Create(mapRoomsWide * kMapRoomWidth, 0);
		surface->DrawLine(leftPoint, rightPoint);
	}
	
	if (activeRoomVisible)
	{
		surface->SetForeColor(StdColors::Red());
		activeRoomRect.right++;
		activeRoomRect.bottom++;
		surface->FrameRect(activeRoomRect);
		InsetRect(&activeRoomRect, 1, 1);
		surface->FrameRect(activeRoomRect);
		surface->SetForeColor(StdColors::Black());
		InsetRect(&activeRoomRect, -1, -1);
	}
	
	surface->SetClipRect(wasClip);
}
#endif

//--------------------------------------------------------------  UpdateMapWindow

void UpdateMapWindow (void)
{
	#ifndef COMPILEDEMO
	if (mapWindow == nil)
		return;
	
	mapHScroll->SetState(mapLeftRoom);
	mapVScroll->SetState(mapTopRoom);

	mapWindow->DrawControls();
	
	SetPortWindowPort(mapWindow);
	// PL_NotYetImplemented_TODO("Resize")
	RedrawMapContents();
	#endif
}

//--------------------------------------------------------------  ResizeMapWindow

void ResizeMapWindow (short newH, short newV)
{
#ifndef COMPILEDEMO
	if ((newH == 0) && (newV == 0))
		return;

	DrawSurface *surface = mapWindow->GetDrawSurface();

	mapRoomsWide = newH / kMapRoomWidth;
	if (mapRoomsWide < 3)
		mapRoomsWide = 3;
	mapRoomsHigh = newV / kMapRoomHeight;
	if (mapRoomsHigh < 3)
		mapRoomsHigh = 3;
	QSetRect(&mapWindowRect, 0, 0, 
			mapRoomsWide * kMapRoomWidth + kMapScrollBarWidth - 2, 
			mapRoomsHigh * kMapRoomHeight + kMapScrollBarWidth - 2);

	surface->SetForeColor(StdColors::White());
	surface->FillRect(mapWindowRect);
	SizeWindow(mapWindow, mapWindowRect.right, mapWindowRect.bottom, true);
	
	mapHScroll->SetMax(kMaxNumRoomsH - mapRoomsWide);
	mapHScroll->SetPosition(Point::Create(0, mapWindowRect.bottom - kMapScrollBarWidth + 2));
	mapHScroll->Resize(mapWindowRect.right - kMapScrollBarWidth + 3, kMapScrollBarWidth);
	mapLeftRoom = mapHScroll->GetState();
	
	mapVScroll->SetMax(kMaxNumRoomsV - mapRoomsHigh);
	mapVScroll->SetPosition(Point::Create(mapWindowRect.right - kMapScrollBarWidth + 2, 0));
	mapVScroll->Resize(kMapScrollBarWidth, mapWindowRect.bottom - kMapScrollBarWidth + 3);
	mapTopRoom = mapVScroll->GetState();
	
	InvalWindowRect(mapWindow, &mapWindowRect);
#endif
}

//--------------------------------------------------------------  OpenMapWindow

void OpenMapWindow (void)
{
#ifndef COMPILEDEMO
	Rect		src, dest;
	
	if (mapWindow == nil)
	{
		CreateNailOffscreen();
		QSetRect(&mapWindowRect, 0, 0, 
				mapRoomsWide * kMapRoomWidth + kMapScrollBarWidth - 2, 
				mapRoomsHigh * kMapRoomHeight + kMapScrollBarWidth - 2);

		const uint16_t windowStyle = PortabilityLayer::WindowStyleFlags::kTitleBar | PortabilityLayer::WindowStyleFlags::kResizable | PortabilityLayer::WindowStyleFlags::kMiniBar;

		PortabilityLayer::WindowDef wdef = PortabilityLayer::WindowDef::Create(mapWindowRect, windowStyle, false, true, 0, 0, PSTR("Map"));
		
		mapWindow = PortabilityLayer::WindowManager::GetInstance()->CreateWindow(wdef);

		if (mapWindow == nil)
			RedAlert(kErrNoMemory);
		
//		if (OptionKeyDown())
//		{
//			isMapH = 3;
//			isMapV = qd.screenBits.bounds.bottom - 100;
//		}
		MoveWindow(mapWindow, isMapH, isMapV, true);

		QSetRect(&wasActiveRoomRect, 0, 0, 1, 1);
		QSetRect(&activeRoomRect, 0, 0, 1, 1);

		GetWindowRect(mapWindow, &dest);
		PortabilityLayer::WindowManager::GetInstance()->PutWindowBehind(mapWindow, kPutInFront);
		PortabilityLayer::WindowManager::GetInstance()->ShowWindow(mapWindow);
//		FlagWindowFloating(mapWindow);	TEMP - use flaoting windows
		
		SetPort((GrafPtr)mapWindow);
		QSetRect(&mapHScrollRect, -1, mapRoomsHigh * kMapRoomHeight, 
				mapRoomsWide * kMapRoomWidth + 1, 
				mapRoomsHigh * kMapRoomHeight + kMapScrollBarWidth);
		QSetRect(&mapVScrollRect, mapRoomsWide * kMapRoomWidth, -1, 
				mapRoomsWide * kMapRoomWidth + kMapScrollBarWidth, 
				mapRoomsHigh * kMapRoomHeight + 1);

		{
			PortabilityLayer::WidgetBasicState state;
			state.m_rect = mapHScrollRect;
			state.m_refConstant = kHScrollRef;
			state.m_window = mapWindow;
			state.m_max = kMaxNumRoomsH - mapRoomsWide;
			state.m_state = mapLeftRoom;
			mapHScroll = PortabilityLayer::ScrollBarWidget::Create(state);
		}

		if (mapHScroll == nil)
			RedAlert(kErrNoMemory);

		{
			PortabilityLayer::WidgetBasicState state;
			state.m_rect = mapVScrollRect;
			state.m_refConstant = kVScrollRef;
			state.m_window = mapWindow;
			state.m_max = kMaxNumRoomsV - mapRoomsHigh;
			state.m_state = mapTopRoom;
			mapVScroll = PortabilityLayer::ScrollBarWidget::Create(state);
		}

		if (mapVScroll == nil)
			RedAlert(kErrNoMemory);
		
		QSetRect(&mapCenterRect, -16, -16, 0, 0);
		QOffsetRect(&mapCenterRect, mapWindowRect.right + 2, 
				mapWindowRect.bottom + 2);
		
		CenterMapOnRoom(thisRoom->suite, thisRoom->floor);
	}
	
	UpdateMapCheckmark(true);
#endif
}

//--------------------------------------------------------------  CloseMapWindow

void CloseMapWindow (void)
{
#ifndef COMPILEDEMO
	CloseThisWindow(&mapWindow);
	UpdateMapCheckmark(false);
#endif
}

//--------------------------------------------------------------  ToggleMapWindow

void ToggleMapWindow (void)
{
#ifndef COMPILEDEMO
	if (mapWindow == nil)
	{
		OpenMapWindow();
		isMapOpen = true;
	}
	else
	{
		CloseMapWindow();
		isMapOpen = false;
	}
#endif
}

//--------------------------------------------------------------  LiveHScrollAction
#ifndef COMPILEDEMO

void LiveHScrollAction (ControlHandle theControl, short thePart)
{
	short		wasValue, newValue;
	
	switch (thePart)
	{
		case kControlUpButtonPart:
		wasValue = GetControlValue(theControl);
		SetControlValue(theControl, wasValue - 1);
		if (GetControlValue(theControl) != wasValue)
		{
			mapLeftRoom = GetControlValue(theControl);
			RedrawMapContents();
		}
		break;
		
		case kControlDownButtonPart:
		wasValue = GetControlValue(theControl);
		SetControlValue(theControl, wasValue + 1);
		if (GetControlValue(theControl) != wasValue)
		{
			mapLeftRoom = GetControlValue(theControl);
			RedrawMapContents();
		}
		break;
		
		case kControlPageUpPart:
		wasValue = GetControlValue(theControl);
		newValue = wasValue - (mapRoomsWide / 2);
		SetControlValue(theControl, newValue);
		if (GetControlValue(theControl) != wasValue)
		{
			mapLeftRoom = GetControlValue(theControl);
			RedrawMapContents();
		}
		break;
		
		case kControlPageDownPart:
		wasValue = GetControlValue(theControl);
		newValue = wasValue + (mapRoomsWide / 2);
		SetControlValue(theControl, newValue);
		if (GetControlValue(theControl) != wasValue)
		{
			mapLeftRoom = GetControlValue(theControl);
			RedrawMapContents();
		}
		break;
		
		case kControlIndicatorPart:
		break;
	}
}
#endif

//--------------------------------------------------------------  LiveVScrollAction
#ifndef COMPILEDEMO

void LiveVScrollAction (ControlHandle theControl, short thePart)
{
	short		wasValue, newValue;
	
	switch (thePart)
	{
		case kControlUpButtonPart:
		wasValue = GetControlValue(theControl);
		SetControlValue(theControl, wasValue - 1);
		if (GetControlValue(theControl) != wasValue)
		{
			mapTopRoom = GetControlValue(theControl);
			RedrawMapContents();
		}
		break;
		
		case kControlDownButtonPart:
		wasValue = GetControlValue(theControl);
		SetControlValue(theControl, wasValue + 1);
		if (GetControlValue(theControl) != wasValue)
		{
			mapTopRoom = GetControlValue(theControl);
			RedrawMapContents();
		}
		break;
		
		case kControlPageUpPart:
		wasValue = GetControlValue(theControl);
		newValue = wasValue - (mapRoomsHigh / 2);
		SetControlValue(theControl, newValue);
		if (GetControlValue(theControl) != wasValue)
		{
			mapTopRoom = GetControlValue(theControl);
			RedrawMapContents();
		}
		break;
		
		case kControlPageDownPart:
		wasValue = GetControlValue(theControl);
		newValue = wasValue + (mapRoomsHigh / 2);
		SetControlValue(theControl, newValue);
		if (GetControlValue(theControl) != wasValue)
		{
			mapTopRoom = GetControlValue(theControl);
			RedrawMapContents();
		}
		break;
		
		case kControlIndicatorPart:
		break;
	}
}
#endif

//--------------------------------------------------------------  HandleMapClick

void HandleMapClick (const GpMouseInputEvent &theEvent)
{
#ifndef COMPILEDEMO
	Rect				aRoom;
	ControlHandle		whichControl;
	Point				wherePt, globalWhere;
	long				controlRef;
	short				whichPart, localH, localV;
	short				roomH, roomV, itsNumber;
	ControlActionUPP	scrollHActionUPP, scrollVActionUPP;
	
	wherePt = Point::Create(theEvent.m_x, theEvent.m_y);
	
	scrollHActionUPP = NewControlActionUPP(LiveHScrollAction);
	scrollVActionUPP = NewControlActionUPP(LiveVScrollAction);
	
	if (mapWindow == nil)
		return;
	
	SetPortWindowPort(mapWindow);
	globalWhere = wherePt;
	wherePt -= mapWindow->TopLeftCoord();
	wherePt.h -= 1;
	wherePt.v -= 1;
	
	whichPart = FindControl(wherePt, mapWindow, &whichControl);
	if (whichPart == 0)				// User clicked in map content area.
	{
		localH = wherePt.h / kMapRoomWidth;
		localV = wherePt.v / kMapRoomHeight;
		
		if ((localH >= mapRoomsWide) || (localV >= mapRoomsHigh))
			return;
		
		roomH = localH + mapLeftRoom;
		roomV = kMapGroundValue - (localV + mapTopRoom);
		
		if (RoomExists(roomH, roomV, &itsNumber))
		{
			CopyRoomToThisRoom(itsNumber);
			DeselectObject();
			ReflectCurrentRoom(false);
			
			if (thisMac.hasDrag)
			{
				SetPortWindowPort(mainWindow);
				QSetRect(&aRoom, 0, 0, kMapRoomWidth, kMapRoomHeight);
				CenterRectOnPoint(&aRoom, globalWhere);
//				if (DragRoom(theEvent, &aRoom, itsNumber))
//				{		// TEMP disabled.
//				}
			}
		}
		else
		{
			if (doBitchDialogs)
			{
				if (QueryNewRoom())
				{
					if (!CreateNewRoom(roomH, roomV))
					{
						YellowAlert(kYellowUnaccounted, 11);
						return;
					}
					else
					{
						DeselectObject();
						ReflectCurrentRoom(false);
					}
				}
				else
					return;
			}
			else
			{
				if (!CreateNewRoom(roomH, roomV))
				{
					YellowAlert(kYellowUnaccounted, 11);
					return;
				}
				else
				{
					DeselectObject();
					ReflectCurrentRoom(false);
				}
			}
		}
	}
	else
	{
		controlRef = GetControlReference(whichControl);
		if (controlRef == kHScrollRef)
		{
			switch (whichPart)
			{
				case kControlUpButtonPart:
				case kControlDownButtonPart:
				case kControlPageUpPart:
				case kControlPageDownPart:
				if (TrackControl(whichControl, wherePt, scrollHActionUPP))
				{
					
				}
				break;
				
				case kControlIndicatorPart:
				if (TrackControl(whichControl, wherePt, nil))
				{
					mapLeftRoom = GetControlValue(whichControl);
					RedrawMapContents();
				}
				break;
			}
		}
		else if (controlRef == kVScrollRef)
		{
			switch (whichPart)
			{
				case kControlUpButtonPart:
				case kControlDownButtonPart:
				case kControlPageUpPart:
				case kControlPageDownPart:
				if (TrackControl(whichControl, wherePt, scrollVActionUPP))
				{
					
				}
				break;
				
				case kControlIndicatorPart:
				if (TrackControl(whichControl, wherePt, nil))
				{
					mapTopRoom = GetControlValue(whichControl);
					RedrawMapContents();
				}
				break;
			}
		}
	}
	
	DisposeControlActionUPP(scrollHActionUPP);
	DisposeControlActionUPP(scrollVActionUPP);
#endif
}

//--------------------------------------------------------------  QueryNewRoom

#ifndef COMPILEDEMO
Boolean QueryNewRoom (void)
{
	short		hitWhat;
	
//	CenterAlert(kNewRoomAlert);
	hitWhat = Alert(kNewRoomAlert, nil);
	if (hitWhat == kYesDoNewRoom)
		return (true);
	else
		return (false);
}
#endif

//--------------------------------------------------------------  CreateNailOffscreen

#ifndef COMPILEDEMO
void CreateNailOffscreen (void)
{
	DrawSurface		*wasCPort;
	PLError_t		theErr;
	
	if (nailSrcMap == nil)
	{
		QSetRect(&nailSrcRect, 0, 0, kMapRoomWidth, kMapRoomHeight * (kNumBackgrounds + 1));
		theErr = CreateOffScreenGWorld(&nailSrcMap, &nailSrcRect, kPreferredPixelFormat);

		LoadGraphic(nailSrcMap, kThumbnailPictID);
	}
}
#endif

//--------------------------------------------------------------  KillNailOffscreen

#ifndef COMPILEDEMO
void KillNailOffscreen (void)
{
	if (nailSrcMap != nil)
	{
//		KillOffScreenPixMap(nailSrcMap);
		DisposeGWorld(nailSrcMap);
		nailSrcMap = nil;
	}
}
#endif

//--------------------------------------------------------------  MoveRoom

void MoveRoom (Point wherePt)
{
	short		localH, localV;
	short		roomH, roomV, itsNumber;
	
	localH = wherePt.h / kMapRoomWidth;
	localV = wherePt.v / kMapRoomHeight;
	
	if ((localH >= mapRoomsWide) || (localV >= mapRoomsHigh))
		return;
	
	roomH = localH + mapLeftRoom;
	roomV = kMapGroundValue - (localV + mapTopRoom);
	
	if (RoomExists(roomH, roomV, &itsNumber))
	{
		
	}
	else
	{
		thisRoom->floor = roomV;
		thisRoom->suite = roomH;
		fileDirty = true;
		UpdateMenus(false);
		RedrawMapContents();
	}
}

