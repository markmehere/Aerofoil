
//============================================================================
//----------------------------------------------------------------------------
//								   Scoreboard.c
//----------------------------------------------------------------------------
//============================================================================


#include "PLNumberFormatting.h"
#include "PLPasStr.h"
#include "Externs.h"
#include "Environ.h"
#include "FontFamily.h"
#include "MenuManager.h"
#include "PLStandardColors.h"
#include "QDPixMap.h"
#include "QDStandardPalette.h"
#include "RectUtils.h"
#include "RenderedFont.h"
#include "ResolveCachingColor.h"


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
	boardWindow->GetDrawSurface()->m_port.SetDirty(PortabilityLayer::QDPortDirtyFlag_Contents);
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
			GetPortBitMapForCopyBits(boardWindow->GetDrawSurface()),
			&boardSrcRect, &boardDestRect, srcCopy);

	MarkScoreboardPortDirty();
	
	QuickBatteryRefresh(false);
	QuickBandsRefresh(false);
	QuickFoilRefresh(false);

	PortabilityLayer::MenuManager::GetInstance()->SetMenuVisible(false);
}

void ClearScoreboard (void)
{
	DrawSurface *surface = boardWindow->GetDrawSurface();

	PortabilityLayer::ResolveCachingColor blackColor(StdColors::Black());
	surface->FillRect(boardWindow->GetSurfaceRect(), blackColor);
	MarkScoreboardPortDirty();
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

	PortabilityLayer::ResolveCachingColor theRGBColor = PortabilityLayer::ResolveCachingColor::FromStandardColor(kGrayBackgroundColor);
	PortabilityLayer::ResolveCachingColor blackColor = StdColors::Black();
	PortabilityLayer::ResolveCachingColor whiteColor = StdColors::White();
	
	surface->FillRect(boardTSrcRect, theRGBColor);
	
	const Point strShadowPoint = Point::Create(1, 10);
	const Point strPoint = Point::Create(0, 9);

	PortabilityLayer::RenderedFont *appFont = GetFont(PortabilityLayer::FontPresets::kApplication12Bold);

	switch (mode)
	{
		case kEscapedTitleMode:
		surface->DrawString(strShadowPoint, PSTR("Hit Delete key if unable to Follow"), blackColor, appFont);
		break;
		
		case kSavingTitleMode:
		surface->DrawString(strShadowPoint, PSTR("Saving Game\xc9"), blackColor, appFont);
		break;
		
		default:
		surface->DrawString(strShadowPoint, thisRoom->name, blackColor, appFont);
		break;
	}

	switch (mode)
	{
		case kEscapedTitleMode:
		surface->DrawString(strPoint, PSTR("Hit Delete key if unable to Follow"), whiteColor, appFont);
		break;
		
		case kSavingTitleMode:
		surface->DrawString(strPoint, PSTR("Saving Game\xc9"), whiteColor, appFont);
		break;
		
		default:
		surface->DrawString(strPoint, thisRoom->name, whiteColor, appFont);
		break;
	}
	
	CopyBits((BitMap *)*GetGWorldPixMap(boardTSrcMap), 
			(BitMap *)*GetGWorldPixMap(boardSrcMap), 
			&boardTSrcRect, &boardTDestRect, srcCopy);
}

//--------------------------------------------------------------  RefreshNumGliders

void RefreshNumGliders (void)
{
	Str255		nGlidersStr;
	long		displayMortals;
	DrawSurface	*surface = boardGSrcMap;

	PortabilityLayer::RenderedFont *appFont = GetFont(PortabilityLayer::FontPresets::kApplication12Bold);

	PortabilityLayer::ResolveCachingColor theRGBColor = PortabilityLayer::ResolveCachingColor::FromStandardColor(kGrayBackgroundColor);
	PortabilityLayer::ResolveCachingColor blackColor = StdColors::Black();
	PortabilityLayer::ResolveCachingColor whiteColor = StdColors::White();

	surface->FillRect(boardGSrcRect, theRGBColor);
	
	displayMortals = mortals;
	if (displayMortals < 0)
		displayMortals = 0;
	NumToString(displayMortals, nGlidersStr);

	const Point shadowPoint = Point::Create(1, 10);
	const Point textPoint = Point::Create(0, 9);

	surface->DrawString(shadowPoint, nGlidersStr, blackColor, appFont);
	surface->DrawString(textPoint, nGlidersStr, whiteColor, appFont);
	
	CopyBits((BitMap *)*GetGWorldPixMap(boardGSrcMap), 
			(BitMap *)*GetGWorldPixMap(boardSrcMap), 
			&boardGSrcRect, &boardGDestRect, srcCopy);
}

//--------------------------------------------------------------  RefreshPoints

void RefreshPoints (void)
{
	Str255		scoreStr;
	DrawSurface	*surface = boardPSrcMap;

	PortabilityLayer::ResolveCachingColor theRGBColor = PortabilityLayer::ResolveCachingColor::FromStandardColor(kGrayBackgroundColor);
	PortabilityLayer::ResolveCachingColor blackColor = StdColors::Black();
	PortabilityLayer::ResolveCachingColor whiteColor = StdColors::White();

	PortabilityLayer::RenderedFont *appFont = GetFont(PortabilityLayer::FontPresets::kApplication12Bold);

	surface->FillRect(boardPSrcRect, theRGBColor);
	
	NumToString(theScore, scoreStr);

	const Point shadowPoint = Point::Create(1, 10);
	const Point textPoint = Point::Create(0, 9);

	surface->DrawString(shadowPoint, scoreStr, blackColor, appFont);
	surface->DrawString(textPoint, scoreStr, whiteColor, appFont);

	CopyBits((BitMap *)*GetGWorldPixMap(boardPSrcMap), 
			(BitMap *)*GetGWorldPixMap(boardSrcMap), 
			&boardPSrcRect, &boardPDestRect, srcCopy);
	
	displayedScore = theScore;
}

//--------------------------------------------------------------  QuickGlidersRefresh

void QuickGlidersRefresh (void)
{
	Str255		nGlidersStr;
	DrawSurface	*surface = boardGSrcMap;

	PortabilityLayer::ResolveCachingColor theRGBColor = PortabilityLayer::ResolveCachingColor::FromStandardColor(kGrayBackgroundColor);
	PortabilityLayer::ResolveCachingColor blackColor = StdColors::Black();
	PortabilityLayer::ResolveCachingColor whiteColor = StdColors::White();

	PortabilityLayer::RenderedFont *appFont = GetFont(PortabilityLayer::FontPresets::kApplication12Bold);

	surface->FillRect(boardGSrcRect, theRGBColor);
	
	NumToString((long)mortals, nGlidersStr);

	const Point shadowPoint = Point::Create(1, 10);
	const Point textPoint = Point::Create(0, 9);

	surface->DrawString(shadowPoint, nGlidersStr, blackColor, appFont);
	surface->DrawString(textPoint, nGlidersStr, whiteColor, appFont);

	CopyBits((BitMap *)*GetGWorldPixMap(boardGSrcMap), 
			GetPortBitMapForCopyBits(boardWindow->GetDrawSurface()),
			&boardGSrcRect, &boardGQDestRect, srcCopy);

	MarkScoreboardPortDirty();
}

//--------------------------------------------------------------  QuickScoreRefresh

void QuickScoreRefresh (void)
{
	Str255		scoreStr;
	DrawSurface	*surface = boardPSrcMap;

	PortabilityLayer::ResolveCachingColor theRGBColor = PortabilityLayer::ResolveCachingColor::FromStandardColor(kGrayBackgroundColor);
	PortabilityLayer::ResolveCachingColor blackColor = StdColors::Black();
	PortabilityLayer::ResolveCachingColor whiteColor = StdColors::White();

	PortabilityLayer::RenderedFont *appFont = GetFont(PortabilityLayer::FontPresets::kApplication12Bold);

	surface->FillRect(boardPSrcRect, theRGBColor);
	
	NumToString(displayedScore, scoreStr);

	const Point shadowPoint = Point::Create(1, 10);
	const Point textPoint = Point::Create(0, 9);

	surface->DrawString(shadowPoint, scoreStr, blackColor, appFont);
	surface->DrawString(textPoint, scoreStr, whiteColor, appFont);
	
	CopyBits((BitMap *)*GetGWorldPixMap(boardPSrcMap), 
			GetPortBitMapForCopyBits(boardWindow->GetDrawSurface()),
			&boardPSrcRect, &boardPQDestRect, srcCopy);

	MarkScoreboardPortDirty();
}

//--------------------------------------------------------------  QuickBatteryRefresh

void QuickBatteryRefresh (Boolean flash)
{
	if ((batteryTotal > 0) && (!flash))
	{
		CopyBits((BitMap *)*GetGWorldPixMap(badgeSrcMap), 
				GetPortBitMapForCopyBits(boardWindow->GetDrawSurface()),
				&badgesBadgesRects[kBatteryBadge], 
				&badgesDestRects[kBatteryBadge], 
				srcCopy);
	}
	else if ((batteryTotal < 0) && (!flash))
	{
		CopyBits((BitMap *)*GetGWorldPixMap(badgeSrcMap), 
				GetPortBitMapForCopyBits(boardWindow->GetDrawSurface()),
				&badgesBadgesRects[kHeliumBadge], 
				&badgesDestRects[kHeliumBadge], 
				srcCopy);
	}
	else
	{
		CopyBits((BitMap *)*GetGWorldPixMap(badgeSrcMap), 
				GetPortBitMapForCopyBits(boardWindow->GetDrawSurface()),
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
				GetPortBitMapForCopyBits(boardWindow->GetDrawSurface()),
				&badgesBadgesRects[kBandsBadge], 
				&badgesDestRects[kBandsBadge], 
				srcCopy);
	}
	else
	{
		CopyBits((BitMap *)*GetGWorldPixMap(badgeSrcMap), 
				GetPortBitMapForCopyBits(boardWindow->GetDrawSurface()),
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
				GetPortBitMapForCopyBits(boardWindow->GetDrawSurface()),
				&badgesBadgesRects[kFoilBadge], 
				&badgesDestRects[kFoilBadge], 
				srcCopy);
	}
	else
	{
		CopyBits((BitMap *)*GetGWorldPixMap(badgeSrcMap), 
				GetPortBitMapForCopyBits(boardWindow->GetDrawSurface()),
				&badgesBlankRects[kFoilBadge], 
				&badgesDestRects[kFoilBadge], 
				srcCopy);
	}

	MarkScoreboardPortDirty();
}
