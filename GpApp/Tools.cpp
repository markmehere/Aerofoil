//============================================================================
//----------------------------------------------------------------------------
//								   Tools.c
//----------------------------------------------------------------------------
//============================================================================


#include "PLTextUtils.h"
#include "PLControlDefinitions.h"
#include "PLPasStr.h"
#include "PLStandardColors.h"
#include "Externs.h"
#include "Environ.h"
#include "FontFamily.h"
#include "PLWidgets.h"
#include "PLPopupMenuWidget.h"
#include "QDPixMap.h"
#include "RectUtils.h"
#include "Utilities.h"
#include "WindowDef.h"
#include "WindowManager.h"


#define kToolsHigh			4
#define kToolsWide			4
#define kTotalTools			16				// kToolsHigh * kToolsWide
#define kPopUpControl		129
#define kFirstBlower		1
#define kLastBlower			15
#define kBlowerBase			1
#define kFirstFurniture		1
#define kLastFurniture		15
#define kFurnitureBase		21
#define kFirstBonus			1
#define kLastBonus			15
#define kBonusBase			41
#define kFirstTransport		1
#define kLastTransport		12
#define kTransportBase		61
#define kFirstSwitch		1
#define kLastSwitch			9
#define kSwitchBase			81
#define kFirstLight			1
#define kLastLight			8
#define kLightBase			101
#define kFirstAppliance		1
#define kLastAppliance		14
#define kApplianceBase		121
#define kFirstEnemy			1
#define kLastEnemy			9
#define kEnemyBase			141
#define kFirstClutter		1
#define kLastClutter		15
#define kClutterBase		161
#define kToolsPictID		1011


void CreateToolsOffscreen (void);
void KillToolsOffscreen (void);
void FrameSelectedTool (DrawSurface *);
void DrawToolName (DrawSurface *);
void DrawToolTiles (DrawSurface *);
void SwitchToolModes (short);


Rect			toolsWindowRect, toolSrcRect, toolTextRect;
Rect			toolRects[kTotalTools];
PortabilityLayer::Widget	*classPopUp;
DrawSurface		*toolSrcMap;
WindowPtr		toolsWindow;
short			isToolsH, isToolsV;
short			toolSelected, toolMode;
short			firstTool, lastTool, objectBase;
Boolean			isToolsOpen;


//==============================================================  Functions
//--------------------------------------------------------------  CreateToolsOffscreen

#ifndef COMPILEDEMO
void CreateToolsOffscreen (void)
{
	DrawSurface		*wasCPort;
	PLError_t		theErr;
	
	if (toolSrcMap == nil)
	{
		QSetRect(&toolSrcRect, 0, 0, 360, 216);
		theErr = CreateOffScreenGWorld(&toolSrcMap, &toolSrcRect, kPreferredPixelFormat);
		LoadGraphic(toolSrcMap, kToolsPictID);
	}
}
#endif

//--------------------------------------------------------------  KillToolsOffscreen

#ifndef COMPILEDEMO
void KillToolsOffscreen (void)
{
	if (toolSrcMap != nil)
	{
		DisposeGWorld(toolSrcMap);
//		KillOffScreenPixMap(toolSrcMap);
		toolSrcMap = nil;
	}
}
#endif

//--------------------------------------------------------------  FrameSelectedTool

#ifndef COMPILEDEMO
void FrameSelectedTool (DrawSurface *surface)
{
	Rect		theRect;
	short		toolIcon;
	
	toolIcon = toolSelected;
	if ((toolMode == kBlowerMode) && (toolIcon >= 7))
	{
		toolIcon--;
	}
	else if ((toolMode == kTransportMode) && (toolIcon >= 7))
	{
		if (toolIcon >= 15)
			toolIcon -= 4;
		else
			toolIcon = ((toolIcon - 7) / 2) + 7;
	}
	
	theRect = toolRects[toolIcon];
	surface->SetForeColor(StdColors::Red());

	surface->FrameRect(theRect);
	InsetRect(&theRect, 1, 1);
	surface->FrameRect(theRect);

	ForeColor(blackColor);
}
#endif

//--------------------------------------------------------------  DrawToolName

#ifndef COMPILEDEMO
void DrawToolName (DrawSurface *surface)
{
	Str255		theString;
	
	if (toolSelected == 0)
		PasStringCopy(PSTR("Selection Tool"), theString);
	else
		GetIndString(theString, kObjectNameStrings, 
				toolSelected + ((toolMode - 1) * 0x0010));

	surface->SetForeColor(StdColors::White());
	surface->FillRect(toolTextRect);
	surface->SetForeColor(StdColors::Black());

	const Point textPoint = Point::Create(toolTextRect.left + 3, toolTextRect.bottom - 6);

	surface->SetApplicationFont(9, PortabilityLayer::FontFamilyFlag_Bold);
	ColorText(surface, textPoint, theString, 171L);
}
#endif

//--------------------------------------------------------------  DrawToolTiles

#ifndef COMPILEDEMO
void DrawToolTiles (DrawSurface *surface)
{
	Rect		srcRect, destRect;
	short		i;
	
	DrawCIcon(surface, 2000, toolRects[0].left, toolRects[0].top);	// Selection Tool
	
	for (i = 0; i < 15; i++)								// Other tools
	{
		QSetRect(&srcRect, 0, 0, 24, 24);
		QSetRect(&destRect, 0, 0, 24, 24);
		
		QOffsetRect(&srcRect, i * 24, (toolMode - 1) * 24);
		QOffsetRect(&destRect, toolRects[i + 1].left + 2, toolRects[i + 1].top + 2);
		
		CopyBits((BitMap *)*GetGWorldPixMap(toolSrcMap), 
				GetPortBitMapForCopyBits(GetWindowPort(toolsWindow)), 
				&srcRect, &destRect, srcCopy);
	}
}
#endif

//--------------------------------------------------------------  EraseSelectedTool

void EraseSelectedTool (void)
{
#ifndef COMPILEDEMO
	DrawSurface *surface = toolsWindow->GetDrawSurface();
	Rect		theRect;
	short		toolIcon;
	
	if (toolsWindow == nil)
		return;
	
	SetPort(&toolsWindow->GetDrawSurface()->m_port);
	
	toolIcon = toolSelected;
	if ((toolMode == kBlowerMode) && (toolIcon >= 7))
	{
		toolIcon--;
	}
	else if ((toolMode == kTransportMode) && (toolIcon >= 7))
	{
		if (toolIcon >= 15)
			toolIcon -= 4;
		else
			toolIcon = ((toolIcon - 7) / 2) + 7;
	}
	
	theRect = toolRects[toolIcon];
	surface->SetForeColor(StdColors::White());
	surface->FrameRect(theRect);
	InsetRect(&theRect, 1, 1);
	surface->FrameRect(theRect);
#endif
}

//--------------------------------------------------------------  SelectTool

void SelectTool (short which)
{
#ifndef COMPILEDEMO
	Rect		theRect;
	short		toolIcon;
	
	if (toolsWindow == nil)
		return;

	DrawSurface *surface = toolsWindow->GetDrawSurface();
	
	toolIcon = which;
	if ((toolMode == kBlowerMode) && (toolIcon >= 7))
	{
		toolIcon--;
	}
	else if ((toolMode == kTransportMode) && (toolIcon >= 7))
	{
		if (toolIcon >= 15)
			toolIcon -= 4;
		else
			toolIcon = ((toolIcon - 7) / 2) + 7;
	}
	
	theRect = toolRects[toolIcon];
	surface->SetForeColor(StdColors::Red());

	surface->FrameRect(theRect);
	InsetRect(&theRect, 1, 1);
	surface->FrameRect(theRect);

	toolSelected = which;
	DrawToolName(surface);
#endif
}

//--------------------------------------------------------------  UpdateToolsWindow

void UpdateToolsWindow (void)
{
#ifndef COMPILEDEMO
	if (toolsWindow == nil)
		return;

	DrawSurface *surface = toolsWindow->GetDrawSurface();
	
	DkGrayForeColor(surface);
	surface->DrawLine(Point::Create(4, 25), Point::Create(112, 25));
	surface->SetForeColor(StdColors::Black());
	
	DrawToolTiles(surface);
	FrameSelectedTool(surface);
	DrawToolName(surface);
#endif
}

//--------------------------------------------------------------  OpenToolsWindow

void OpenToolsWindow (void)
{
#ifndef COMPILEDEMO
	Rect		src, dest;
	Point		globalMouse;
	short		h, v;

	PortabilityLayer::WindowManager *wm = PortabilityLayer::WindowManager::GetInstance();
	
	if (toolsWindow == nil)
	{
		QSetRect(&toolsWindowRect, 0, 0, 116, 152);		// 143
		QSetRect(&toolTextRect, 0, 0, 116, 12);
		InsetRect(&toolTextRect, -1, -1);
		QOffsetRect(&toolTextRect, 0, 157 - 15);

		{
			const uint16_t windowStyle = PortabilityLayer::WindowStyleFlags::kTitleBar | PortabilityLayer::WindowStyleFlags::kMiniBar | PortabilityLayer::WindowStyleFlags::kCloseBox;

			PortabilityLayer::WindowDef wdef = PortabilityLayer::WindowDef::Create(toolsWindowRect, windowStyle, false, 0, 0, PSTR("Tools"));
			toolsWindow = wm->CreateWindow(wdef);
		}
		
		if (toolsWindow == nil)
			RedAlert(kErrNoMemory);
		
//		if (OptionKeyDown())
//		{
//			isToolsH = qd.screenBits.bounds.right - 120;
//			isToolsV = 35;
//		}
		MoveWindow(toolsWindow, isToolsH, isToolsV, true);

		GetWindowRect(toolsWindow, &dest);
		wm->PutWindowBehind(toolsWindow, wm->GetPutInFrontSentinel());
		wm->ShowWindow(toolsWindow);
//		FlagWindowFloating(toolsWindow);	TEMP - use flaoting windows
		HiliteAllWindows();

		{
			PortabilityLayer::WidgetBasicState state;
			state.m_min = 1;
			state.m_max = 3;
			state.m_rect = Rect::Create(2, 4, 22, 112);
			state.m_state = 1;
			state.m_resID = 141;
			state.m_window = toolsWindow;

			classPopUp = PortabilityLayer::PopupMenuWidget::Create(state);
		}

		toolsWindow->DrawControls();

		if (classPopUp == nil)
			RedAlert(kErrFailedResourceLoad);
		
		classPopUp->SetState(toolMode);
		
		for (v = 0; v < kToolsHigh; v++)
			for (h = 0; h < kToolsWide; h++)
			{
				QSetRect(&toolRects[(v * kToolsWide) + h], 2, 29, 30, 57);
				QOffsetRect(&toolRects[(v * kToolsWide) + h], h * 28, v * 28);
			}
		
		CreateToolsOffscreen();
		
		SwitchToolModes(toolMode);
		toolSelected = kSelectTool;
	}
	
	UpdateToolsCheckmark(true);

	UpdateToolsWindow();
#endif
}

//--------------------------------------------------------------  CloseToolsWindow

void CloseToolsWindow (void)
{
#ifndef COMPILEDEMO
	CloseThisWindow(&toolsWindow);
	KillToolsOffscreen();
	UpdateToolsCheckmark(false);
#endif
}

//--------------------------------------------------------------  ToggleToolsWindow

void ToggleToolsWindow (void)
{
#ifndef COMPILEDEMO
	if (toolsWindow == nil)
	{
		OpenToolsWindow();
		isToolsOpen = true;
	}
	else
	{
		CloseToolsWindow();
		isToolsOpen = true;
	}
#endif
}

//--------------------------------------------------------------  SwitchToolModes

#ifndef COMPILEDEMO
void SwitchToolModes (short newMode)
{
	if (toolsWindow == nil)
		return;
	
	SelectTool(kSelectTool);
	switch (newMode)
	{
		case kBlowerMode:
		firstTool = kFirstBlower;
		lastTool = kLastBlower;
		objectBase = kBlowerBase;
		break;
		
		case kFurnitureMode:
		firstTool = kFirstFurniture;
		lastTool = kLastFurniture;
		objectBase = kFurnitureBase;
		break;
		
		case kBonusMode:
		firstTool = kFirstBonus;
		lastTool = kLastBonus;
		objectBase = kBonusBase;
		break;
		
		case kTransportMode:
		firstTool = kFirstTransport;
		lastTool = kLastTransport;
		objectBase = kTransportBase;
		break;
		
		case kSwitchMode:
		firstTool = kFirstSwitch;
		lastTool = kLastSwitch;
		objectBase = kSwitchBase;
		break;
		
		case kLightMode:
		firstTool = kFirstLight;
		lastTool = kLastLight;
		objectBase = kLightBase;
		break;
		
		case kApplianceMode:
		firstTool = kFirstAppliance;
		lastTool = kLastAppliance;
		objectBase = kApplianceBase;
		break;
		
		case kEnemyMode:
		firstTool = kFirstEnemy;
		lastTool = kLastEnemy;
		objectBase = kEnemyBase;
		break;
		
		case kClutterMode:
		firstTool = kFirstClutter;
		lastTool = kLastClutter;
		objectBase = kClutterBase;
		break;
	}
	
	toolMode = newMode;
}
#endif

//--------------------------------------------------------------  HandleToolsClick

void HandleToolsClick (Point wherePt)
{
#ifndef COMPILEDEMO
	PortabilityLayer::Widget	*theControl;
	short			i, part, newMode, toolIcon;
	
	if (toolsWindow == nil)
		return;
	
	SetPortWindowPort(toolsWindow);
	wherePt -= toolsWindow->TopLeftCoord();
	
	part = FindControl(wherePt, toolsWindow, &theControl);
	if ((theControl != nil) && (part != 0))
	{
		part = theControl->Capture(wherePt, nullptr);
		if (part != 0)
		{
			newMode = theControl->GetState();
			if (newMode != toolMode)
			{
				EraseSelectedTool();
				SwitchToolModes(newMode);
				UpdateToolsWindow();
			}
		}
	}
	else
	{
		for (i = 0; i < kTotalTools; i++)
			if ((toolRects[i].Contains(wherePt)) && (i <= lastTool))
			{
				EraseSelectedTool();
				toolIcon = i;
				if ((toolMode == kBlowerMode) && (toolIcon >= 7))
				{
					toolIcon++;
				}
				if ((toolMode == kTransportMode) && (toolIcon >= 7))
				{
					if (toolIcon >= 11)
						toolIcon += 4;
					else
						toolIcon = ((toolIcon - 7) * 2) + 7;
				}
				SelectTool(toolIcon);
				break;
			}
	}
#endif
}

//--------------------------------------------------------------  NextToolMode

void NextToolMode (void)
{
#ifndef COMPILEDEMO
	if (toolsWindow == nil)
		return;
	
	if ((theMode == kEditMode) && (toolMode < kClutterMode))
	{
		EraseSelectedTool();
		toolMode++;
		classPopUp->SetState(toolMode);
		SwitchToolModes(toolMode);
		toolSelected = kSelectTool;
	}
#endif
}

//--------------------------------------------------------------  PrevToolMode

void PrevToolMode (void)
{
#ifndef COMPILEDEMO
	if (toolsWindow == nil)
		return;
	
	if ((theMode == kEditMode) && (toolMode > kBlowerMode))
	{
		EraseSelectedTool();
		toolMode--;
		classPopUp->SetState(toolMode);
		SwitchToolModes(toolMode);
		toolSelected = kSelectTool;
	}
#endif
}

//--------------------------------------------------------------  SetSpecificToolMode

void SetSpecificToolMode (short modeToSet)
{
#ifndef COMPILEDEMO
	if ((toolsWindow == nil) || (theMode != kEditMode))
		return;
	
	EraseSelectedTool();
	toolMode = modeToSet;
	classPopUp->SetState(toolMode);
	SwitchToolModes(toolMode);
	toolSelected = kSelectTool;
#endif
}

