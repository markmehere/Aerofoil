//============================================================================
//----------------------------------------------------------------------------
//									 Map.c
//----------------------------------------------------------------------------
//============================================================================


#include "PLControlDefinitions.h"
#include "PLResources.h"
#include "PLPasStr.h"
#include "PLStandardColors.h"
#include "DialogManager.h"
#include "Externs.h"
#include "Environ.h"
#include "House.h"
#include "PLScrollBarWidget.h"
#include "PLWidgets.h"
#include "WindowDef.h"
#include "WindowManager.h"
#include "QDPixMap.h"
#include "RectUtils.h"
#include "ResolveCachingColor.h"
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
void LiveHScrollAction (PortabilityLayer::Widget *theControl, int thePart);
void LiveVScrollAction (PortabilityLayer::Widget *theControl, int thePart);
Boolean QueryNewRoom (void);
void CreateNailOffscreen (void);
void KillNailOffscreen (void);
void KeepWindowInBounds (Window *window);

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
	UpdateMapWindow();
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

				if (type > kNumBackgrounds)		// Do a "pretty" thumbnail.
				{
					LoadGraphicPlus(surface, type + kBaseBackgroundID, aRoom);
				}
				else
				{
					QSetRect(&src, 0, 0, kMapRoomWidth, kMapRoomHeight);
					QOffsetRect(&src, 0, type * kMapRoomHeight);
					CopyBits((BitMap *)*GetGWorldPixMap(nailSrcMap), 
							GetPortBitMapForCopyBits(mapWindow->GetDrawSurface()),
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
				PortabilityLayer::ResolveCachingColor whiteColor = StdColors::White();
				surface->FillRect(aRoom, whiteColor);

				PortabilityLayer::ResolveCachingColor overlayColor;
				if (i >= groundLevel)
					overlayColor = StdColors::Green();
				else
					overlayColor = StdColors::Blue();

				Pattern dummyPat;
				surface->FillRectWithMaskPattern8x8(aRoom, *GetQDGlobalsGray(&dummyPat), overlayColor);
			}
		}
	}

	PortabilityLayer::ResolveCachingColor blackColor = StdColors::Black();
	
	for (i = 1; i < mapRoomsWide; i++)
	{
		const Point upperPoint = Point::Create(i * kMapRoomWidth, 0);
		const Point lowerPoint = Point::Create(upperPoint.h, upperPoint.v + mapRoomsHigh * kMapRoomHeight);
		surface->DrawLine(upperPoint, lowerPoint, blackColor);
	}
	
	for (i = 1; i < mapRoomsHigh; i++)
	{
		const Point leftPoint = Point::Create(0, i * kMapRoomHeight);
		const Point rightPoint = leftPoint + Point::Create(mapRoomsWide * kMapRoomWidth, 0);
		surface->DrawLine(leftPoint, rightPoint, blackColor);
	}
	
	if (activeRoomVisible)
	{
		PortabilityLayer::ResolveCachingColor redColor = StdColors::Red();

		activeRoomRect.right++;
		activeRoomRect.bottom++;
		surface->FrameRect(activeRoomRect, redColor);
		InsetRect(&activeRoomRect, 1, 1);
		surface->FrameRect(activeRoomRect, redColor);
		InsetRect(&activeRoomRect, -1, -1);
	}
	
	surface->SetClipRect(wasClip);
}
#endif


//--------------------------------------------------------------  UpdateMapWindow
void DrawMapResizeBox(void)
{
	DrawSurface *surface = mapWindow->GetDrawSurface();

	const Rect windowRect = surface->m_port.GetRect();
	Rect growBoxRect = Rect::Create(windowRect.bottom - 14, windowRect.right - 14, windowRect.bottom, windowRect.right);

	PortabilityLayer::ResolveCachingColor backgroundColor = PortabilityLayer::RGBAColor::Create(204, 204, 204, 255);
	surface->FillRect(growBoxRect, backgroundColor);

	PortabilityLayer::ResolveCachingColor blackColor = StdColors::Black();
	surface->FillRect(Rect::Create(growBoxRect.top + 2, growBoxRect.left + 2, growBoxRect.top + 3, growBoxRect.left + 6), blackColor);
	surface->FillRect(Rect::Create(growBoxRect.top + 3, growBoxRect.left + 2, growBoxRect.top + 6, growBoxRect.left + 3), blackColor);

	surface->FillRect(Rect::Create(growBoxRect.top + 8, growBoxRect.left + 11, growBoxRect.top + 12, growBoxRect.left + 12), blackColor);
	surface->FillRect(Rect::Create(growBoxRect.top + 11, growBoxRect.left + 8, growBoxRect.top + 12, growBoxRect.left + 11), blackColor);

	for (int i = 0; i < 7; i++)
		surface->FillRect(Rect::Create(growBoxRect.top + 3 + i, growBoxRect.left + 3 + i, growBoxRect.top + 5 + i, growBoxRect.left + 5 + i), blackColor);
}

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

	DrawMapResizeBox();
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
			mapRoomsWide * kMapRoomWidth + kMapScrollBarWidth - 1, 
			mapRoomsHigh * kMapRoomHeight + kMapScrollBarWidth - 1);

	PortabilityLayer::ResolveCachingColor whiteColor = StdColors::White();
	surface->FillRect(mapWindowRect, whiteColor);
	SizeWindow(mapWindow, mapWindowRect.right, mapWindowRect.bottom, true);
	
	mapHScroll->SetMax(kMaxNumRoomsH - mapRoomsWide);
	mapHScroll->SetPosition(Point::Create(-1, mapWindowRect.bottom - kMapScrollBarWidth + 1));
	mapHScroll->Resize(mapWindowRect.right - kMapScrollBarWidth + 3, kMapScrollBarWidth);
	mapLeftRoom = mapHScroll->GetState();
	
	mapVScroll->SetMax(kMaxNumRoomsV - mapRoomsHigh);
	mapVScroll->SetPosition(Point::Create(mapWindowRect.right - kMapScrollBarWidth + 1, -1));
	mapVScroll->Resize(kMapScrollBarWidth, mapWindowRect.bottom - kMapScrollBarWidth + 3);
	mapTopRoom = mapVScroll->GetState();

	UpdateMapWindow();
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
				mapRoomsWide * kMapRoomWidth + kMapScrollBarWidth - 1, 
				mapRoomsHigh * kMapRoomHeight + kMapScrollBarWidth - 1);

		const uint16_t windowStyle = PortabilityLayer::WindowStyleFlags::kTitleBar | PortabilityLayer::WindowStyleFlags::kResizable | PortabilityLayer::WindowStyleFlags::kMiniBar | PortabilityLayer::WindowStyleFlags::kCloseBox;

		PortabilityLayer::WindowDef wdef = PortabilityLayer::WindowDef::Create(mapWindowRect, windowStyle, false, 0, 0, PSTR("Map"));
		
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
		
		SetPort(mapWindow->GetDrawSurface());
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
			mapHScroll = PortabilityLayer::ScrollBarWidget::Create(state, nullptr);
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
			mapVScroll = PortabilityLayer::ScrollBarWidget::Create(state, nullptr);
		}

		if (mapVScroll == nil)
			RedAlert(kErrNoMemory);
		
		QSetRect(&mapCenterRect, -16, -16, 0, 0);
		QOffsetRect(&mapCenterRect, mapWindowRect.right + 2, 
				mapWindowRect.bottom + 2);
		
		CenterMapOnRoom(thisRoom->suite, thisRoom->floor);

		UpdateMapWindow();

		KeepWindowInBounds(mapWindow);
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

void LiveHScrollAction (PortabilityLayer::Widget *theControl, int thePart)
{
	short		wasValue, newValue;
	
	switch (thePart)
	{
		case kControlUpButtonPart:
		wasValue = theControl->GetState();
		theControl->SetState(wasValue - 1);
		if (theControl->GetState() != wasValue)
		{
			mapLeftRoom = theControl->GetState();
			RedrawMapContents();
		}
		break;
		
		case kControlDownButtonPart:
		wasValue = theControl->GetState();
		theControl->SetState(wasValue + 1);
		if (theControl->GetState() != wasValue)
		{
			mapLeftRoom = theControl->GetState();
			RedrawMapContents();
		}
		break;
		
		case kControlPageUpPart:
		wasValue = theControl->GetState();
		newValue = wasValue - (mapRoomsWide / 2);
		theControl->SetState(newValue);
		if (theControl->GetState() != wasValue)
		{
			mapLeftRoom = theControl->GetState();
			RedrawMapContents();
		}
		break;
		
		case kControlPageDownPart:
		wasValue = theControl->GetState();
		newValue = wasValue + (mapRoomsWide / 2);
		theControl->SetState(newValue);
		if (theControl->GetState() != wasValue)
		{
			mapLeftRoom = theControl->GetState();
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

void LiveVScrollAction (PortabilityLayer::Widget *theControl, int thePart)
{
	short		wasValue, newValue;
	
	switch (thePart)
	{
		case kControlUpButtonPart:
		wasValue = theControl->GetState();
		theControl->SetState(wasValue - 1);
		if (theControl->GetState() != wasValue)
		{
			mapTopRoom = theControl->GetState();
			RedrawMapContents();
		}
		break;
		
		case kControlDownButtonPart:
		wasValue = theControl->GetState();
		theControl->SetState(wasValue + 1);
		if (theControl->GetState() != wasValue)
		{
			mapTopRoom = theControl->GetState();
			RedrawMapContents();
		}
		break;
		
		case kControlPageUpPart:
		wasValue = theControl->GetState();
		newValue = wasValue - (mapRoomsHigh / 2);
		theControl->SetState(newValue);
		if (theControl->GetState() != wasValue)
		{
			mapTopRoom = theControl->GetState();
			RedrawMapContents();
		}
		break;
		
		case kControlPageDownPart:
		wasValue = theControl->GetState();
		newValue = wasValue + (mapRoomsHigh / 2);
		theControl->SetState(newValue);
		if (theControl->GetState() != wasValue)
		{
			mapTopRoom = theControl->GetState();
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
	PortabilityLayer::Widget	*whichControl = nullptr;
	Point				wherePt, globalWhere;
	long				controlRef;
	short				whichPart, localH, localV;
	short				roomH, roomV, itsNumber;
	
	wherePt = Point::Create(theEvent.m_x, theEvent.m_y);
	
	if (mapWindow == nil)
		return;
	
	SetPortWindowPort(mapWindow);
	globalWhere = wherePt;
	wherePt -= mapWindow->GetTopLeftCoord();
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
		controlRef = whichControl->GetReferenceConstant();
		if (controlRef == kHScrollRef)
		{
			switch (whichPart)
			{
				case kControlUpButtonPart:
				case kControlDownButtonPart:
				case kControlPageUpPart:
				case kControlPageDownPart:
				whichControl->Capture(wherePt, LiveHScrollAction);
				break;
				
				case kControlIndicatorPart:
				if (whichControl->Capture(wherePt, nil))
				{
					mapLeftRoom = whichControl->GetState();
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
				whichControl->Capture(wherePt, LiveVScrollAction);
				break;
				
				case kControlIndicatorPart:
				if (whichControl->Capture(wherePt, nil))
				{
					mapTopRoom = whichControl->GetState();
					RedrawMapContents();
				}
				break;
			}
		}
	}
#endif
}

//--------------------------------------------------------------  QueryNewRoom

#ifndef COMPILEDEMO
Boolean QueryNewRoom (void)
{
	short		hitWhat;
	
//	CenterAlert(kNewRoomAlert);
	hitWhat = PortabilityLayer::DialogManager::GetInstance()->DisplayAlert(kNewRoomAlert, nullptr);
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

