
//============================================================================
//----------------------------------------------------------------------------
//								   Scoreboard.c
//----------------------------------------------------------------------------
//============================================================================


#include "PLNumberFormatting.h"
#include "PLPasStr.h"
#include "Externs.h"
#include "Environ.h"
#include "MenuManager.h"
#include "QDPixMap.h"
#include "QDStandardPalette.h"
#include "RectUtils.h"


#define kGrayBackgroundColor	251
#define kGrayBackgroundColor4	10
#define kFoilBadge				0
#define kBandsBadge				1
#define kBatteryBadge			2
#define kHeliumBadge			3
#define kScoreRollAmount		13


void RefreshRoomTitle (short);
void RefreshNumGliders (void);
void RefreshPoints (void);


Rect		boardSrcRect, badgeSrcRect, boardDestRect;
DrawSurface	*boardSrcMap, *badgeSrcMap;
Rect		boardTSrcRect, boardTDestRect;
DrawSurface	*boardTSrcMap;
Rect		boardGSrcRect, boardGDestRect;
DrawSurface	*boardGSrcMap;
Rect		boardPSrcRect, boardPDestRect;
Rect		boardPQDestRect, boardGQDestRect;
Rect		badgesBlankRects[4], badgesBadgesRects[4];
Rect		badgesDestRects[4];
DrawSurface	*boardPSrcMap;
long		displayedScore;
short		wasScoreboardTitleMode;
Boolean		doRollScore;

extern	Rect		localRoomsDest[], justRoomsRect;
extern	long		gameFrame;
extern	short		numNeighbors, otherPlayerEscaped;
extern	Boolean		evenFrame, onePlayerLeft;


//==============================================================  Functions
//--------------------------------------------------------------  MarkScoreboardPortDirty

void MarkScoreboardPortDirty(void)
{
	GetWindowPort(boardWindow)->m_port.SetDirty(PortabilityLayer::QDPortDirtyFlag_Contents);
}

//--------------------------------------------------------------  RefreshScoreboard

void RefreshScoreboard (SInt16 mode)
{
	wasScoreboardTitleMode = mode;
	doRollScore = true;
	
	RefreshRoomTitle(mode);
	RefreshNumGliders();
	RefreshPoints();
	
	CopyBits((BitMap *)*GetGWorldPixMap(boardSrcMap), 
			GetPortBitMapForCopyBits(GetWindowPort(boardWindow)),
			&boardSrcRect, &boardDestRect, srcCopy);

	MarkScoreboardPortDirty();
	
	QuickBatteryRefresh(false);
	QuickBandsRefresh(false);
	QuickFoilRefresh(false);

	PortabilityLayer::MenuManager::GetInstance()->SetMenuVisible(false);
}

//--------------------------------------------------------------  HandleDynamicScoreboard

 void HandleDynamicScoreboard (void)
 {
 	#define		kFoilLow		2		// 25%
 	#define		kBatteryLow		17		// 25%
 	#define		kHeliumLow		-38		// 25%
 	#define		kBandsLow		2		// 25%
 	long		whosTurn;
 	
 	if (theScore > displayedScore)
	{
		if (doRollScore)
		{
			displayedScore += kScoreRollAmount;
			if (displayedScore > theScore)
				displayedScore = theScore;
		}
		else
			displayedScore = theScore;
		
		PlayPrioritySound(kScoreTikSound, kScoreTikPriority);
		QuickScoreRefresh();
	}
	
	whosTurn = gameFrame & 0x00000007;
	switch (whosTurn)
	{
		case 0:		// show foil
		if ((foilTotal > 0) && (foilTotal < kFoilLow))
			QuickFoilRefresh(false);
		break;
		
		case 1:		// hide battery
		if ((batteryTotal > 0) && (batteryTotal < kBatteryLow))
			QuickBatteryRefresh(true);
		else if ((batteryTotal < 0) && (batteryTotal > kHeliumLow))
			QuickBatteryRefresh(true);
		break;
		
		case 2:		// show rubber bands
		if ((bandsTotal > 0) && (bandsTotal < kBandsLow))
			QuickBandsRefresh(false);
		break;
		
		case 4:		// show battery
		if ((batteryTotal > 0) && (batteryTotal < kBatteryLow))
			QuickBatteryRefresh(false);
		else if ((batteryTotal < 0) && (batteryTotal > kHeliumLow))
			QuickBatteryRefresh(false);
		break;
		
		case 5:		// hide foil
		if ((foilTotal > 0) && (foilTotal < kFoilLow))
			QuickFoilRefresh(true);
		break;
		
		case 7:		// hide rubber bands
		if ((bandsTotal > 0) && (bandsTotal < kBandsLow))
			QuickBandsRefresh(true);
		break;
	}
 }
 
//--------------------------------------------------------------  RefreshRoomTitle

void RefreshRoomTitle (short mode)
{
	DrawSurface *surface = boardTSrcMap;

	PortabilityLayer::RGBAColor	theRGBColor, wasColor;
	
	wasColor = surface->GetForeColor();
	theRGBColor = PortabilityLayer::StandardPalette::GetInstance()->GetColors()[kGrayBackgroundColor];
	surface->SetForeColor(theRGBColor);
	surface->FillRect(boardTSrcRect);
	surface->SetForeColor(wasColor);
	
	const Point strShadowPoint = Point::Create(1, 10);
	const Point strPoint = Point::Create(0, 9);

	surface->SetForeColor(PortabilityLayer::RGBAColor::Create(0, 0, 0, 255));

	switch (mode)
	{
		case kEscapedTitleMode:
		surface->DrawString(strShadowPoint, PSTR("Hit Delete key if unable to Follow"), true);
		break;
		
		case kSavingTitleMode:
		surface->DrawString(strShadowPoint, PSTR("Saving GameÉ"), true);
		break;
		
		default:
		surface->DrawString(strShadowPoint, thisRoom->name, true);
		break;
	}

	surface->SetForeColor(PortabilityLayer::RGBAColor::Create(255, 255, 255, 255));

	switch (mode)
	{
		case kEscapedTitleMode:
		surface->DrawString(strPoint, PSTR("Hit Delete key if unable to Follow"), true);
		break;
		
		case kSavingTitleMode:
		surface->DrawString(strPoint, PSTR("Saving GameÉ"), true);
		break;
		
		default:
		surface->DrawString(strPoint, thisRoom->name, true);
		break;
	}
	
	CopyBits((BitMap *)*GetGWorldPixMap(boardTSrcMap), 
			(BitMap *)*GetGWorldPixMap(boardSrcMap), 
			&boardTSrcRect, &boardTDestRect, srcCopy);
}

//--------------------------------------------------------------  RefreshNumGliders

void RefreshNumGliders (void)
{
	PortabilityLayer::RGBAColor	theRGBColor, wasColor;
	Str255		nGlidersStr;
	long		displayMortals;
	DrawSurface	*surface = boardGSrcMap;

	theRGBColor = PortabilityLayer::StandardPalette::GetInstance()->GetColors()[kGrayBackgroundColor];

	wasColor = surface->GetForeColor();
	surface->SetForeColor(theRGBColor);
	surface->FillRect(boardGSrcRect);
	
	displayMortals = mortals;
	if (displayMortals < 0)
		displayMortals = 0;
	NumToString(displayMortals, nGlidersStr);

	const Point shadowPoint = Point::Create(1, 10);
	const Point textPoint = Point::Create(0, 9);

	surface->SetForeColor(PortabilityLayer::RGBAColor::Create(0, 0, 0, 255));
	surface->DrawString(shadowPoint, nGlidersStr, true);
	
	surface->SetForeColor(PortabilityLayer::RGBAColor::Create(255, 255, 255, 255));
	surface->DrawString(textPoint, nGlidersStr, true);
	
	CopyBits((BitMap *)*GetGWorldPixMap(boardGSrcMap), 
			(BitMap *)*GetGWorldPixMap(boardSrcMap), 
			&boardGSrcRect, &boardGDestRect, srcCopy);
}

//--------------------------------------------------------------  RefreshPoints

void RefreshPoints (void)
{
	PortabilityLayer::RGBAColor	theRGBColor, wasColor;
	Str255		scoreStr;
	DrawSurface	*surface = boardPSrcMap;

	theRGBColor = PortabilityLayer::StandardPalette::GetInstance()->GetColors()[kGrayBackgroundColor];

	surface->SetForeColor(theRGBColor);
	surface->FillRect(boardPSrcRect);
	
	NumToString(theScore, scoreStr);

	const Point shadowPoint = Point::Create(1, 10);
	const Point textPoint = Point::Create(0, 9);

	surface->SetForeColor(PortabilityLayer::RGBAColor::Create(0, 0, 0, 255));
	surface->DrawString(shadowPoint, scoreStr, true);
	
	surface->SetForeColor(PortabilityLayer::RGBAColor::Create(255, 255, 255, 255));
	surface->DrawString(textPoint, scoreStr, true);

	CopyBits((BitMap *)*GetGWorldPixMap(boardPSrcMap), 
			(BitMap *)*GetGWorldPixMap(boardSrcMap), 
			&boardPSrcRect, &boardPDestRect, srcCopy);
	
	displayedScore = theScore;
}

//--------------------------------------------------------------  QuickGlidersRefresh

void QuickGlidersRefresh (void)
{
	PortabilityLayer::RGBAColor	theRGBColor, wasColor;
	Str255		nGlidersStr;
	DrawSurface	*surface = boardGSrcMap;

	theRGBColor = PortabilityLayer::StandardPalette::GetInstance()->GetColors()[kGrayBackgroundColor];

	surface->SetForeColor(theRGBColor);
	surface->FillRect(boardGSrcRect);
	
	NumToString((long)mortals, nGlidersStr);

	const Point shadowPoint = Point::Create(1, 10);
	const Point textPoint = Point::Create(0, 9);

	surface->SetForeColor(PortabilityLayer::RGBAColor::Create(0, 0, 0, 255));
	surface->DrawString(shadowPoint, nGlidersStr, true);
	
	surface->SetForeColor(PortabilityLayer::RGBAColor::Create(255, 255, 255, 255));
	surface->DrawString(textPoint, nGlidersStr, true);

	CopyBits((BitMap *)*GetGWorldPixMap(boardGSrcMap), 
			GetPortBitMapForCopyBits(GetWindowPort(boardWindow)),
			&boardGSrcRect, &boardGQDestRect, srcCopy);

	MarkScoreboardPortDirty();
}

//--------------------------------------------------------------  QuickScoreRefresh

void QuickScoreRefresh (void)
{
	PortabilityLayer::RGBAColor	theRGBColor, wasColor;
	Str255		scoreStr;
	DrawSurface	*surface = boardPSrcMap;

	theRGBColor = PortabilityLayer::StandardPalette::GetInstance()->GetColors()[kGrayBackgroundColor];
	surface->SetForeColor(theRGBColor);
	surface->FillRect(boardPSrcRect);
	
	NumToString(displayedScore, scoreStr);

	const Point shadowPoint = Point::Create(1, 10);
	const Point textPoint = Point::Create(0, 9);

	surface->SetForeColor(PortabilityLayer::RGBAColor::Create(0, 0, 0, 255));
	surface->DrawString(shadowPoint, scoreStr, true);

	surface->SetForeColor(PortabilityLayer::RGBAColor::Create(255, 255, 255, 255));
	surface->DrawString(textPoint, scoreStr, true);
	
	CopyBits((BitMap *)*GetGWorldPixMap(boardPSrcMap), 
			GetPortBitMapForCopyBits(GetWindowPort(boardWindow)),
			&boardPSrcRect, &boardPQDestRect, srcCopy);

	MarkScoreboardPortDirty();
}

//--------------------------------------------------------------  QuickBatteryRefresh

void QuickBatteryRefresh (Boolean flash)
{
	if ((batteryTotal > 0) && (!flash))
	{
		CopyBits((BitMap *)*GetGWorldPixMap(badgeSrcMap), 
				GetPortBitMapForCopyBits(GetWindowPort(boardWindow)),
				&badgesBadgesRects[kBatteryBadge], 
				&badgesDestRects[kBatteryBadge], 
				srcCopy);
	}
	else if ((batteryTotal < 0) && (!flash))
	{
		CopyBits((BitMap *)*GetGWorldPixMap(badgeSrcMap), 
				GetPortBitMapForCopyBits(GetWindowPort(boardWindow)),
				&badgesBadgesRects[kHeliumBadge], 
				&badgesDestRects[kHeliumBadge], 
				srcCopy);
	}
	else
	{
		CopyBits((BitMap *)*GetGWorldPixMap(badgeSrcMap), 
				GetPortBitMapForCopyBits(GetWindowPort(boardWindow)),
				&badgesBlankRects[kBatteryBadge], 
				&badgesDestRects[kBatteryBadge], 
				srcCopy);
	}

	MarkScoreboardPortDirty();
}

//--------------------------------------------------------------  QuickBandsRefresh

void QuickBandsRefresh (Boolean flash)
{
	if ((bandsTotal > 0) && (!flash))
	{
		CopyBits((BitMap *)*GetGWorldPixMap(badgeSrcMap), 
				GetPortBitMapForCopyBits(GetWindowPort(boardWindow)),
				&badgesBadgesRects[kBandsBadge], 
				&badgesDestRects[kBandsBadge], 
				srcCopy);
	}
	else
	{
		CopyBits((BitMap *)*GetGWorldPixMap(badgeSrcMap), 
				GetPortBitMapForCopyBits(GetWindowPort(boardWindow)),
				&badgesBlankRects[kBandsBadge], 
				&badgesDestRects[kBandsBadge], 
				srcCopy);
	}

	MarkScoreboardPortDirty();
}

//--------------------------------------------------------------  QuickFoilRefresh

void QuickFoilRefresh (Boolean flash)
{
	if ((foilTotal > 0) && (!flash))
	{
		CopyBits((BitMap *)*GetGWorldPixMap(badgeSrcMap), 
				GetPortBitMapForCopyBits(GetWindowPort(boardWindow)),
				&badgesBadgesRects[kFoilBadge], 
				&badgesDestRects[kFoilBadge], 
				srcCopy);
	}
	else
	{
		CopyBits((BitMap *)*GetGWorldPixMap(badgeSrcMap), 
				GetPortBitMapForCopyBits(GetWindowPort(boardWindow)),
				&badgesBlankRects[kFoilBadge], 
				&badgesDestRects[kFoilBadge], 
				srcCopy);
	}

	MarkScoreboardPortDirty();
}
