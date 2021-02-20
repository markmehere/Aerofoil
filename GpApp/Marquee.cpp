//============================================================================
//----------------------------------------------------------------------------
//									Marquee.c
//----------------------------------------------------------------------------
//============================================================================


#include "Externs.h"
#include "IGpDisplayDriver.h"
#include "InputManager.h"
#include "Marquee.h"
#include "Objects.h"
#include "ObjectEdit.h"
#include "RectUtils.h"

#include "PLDrivers.h"

#include <assert.h>
#include <algorithm>


#define	kMarqueePatListID		128
#define kHandleSideLong			9


void DrawGliderMarquee (void);
void DrawMarquee (DrawSurface *surface, const uint8_t *pattern);


marquee		theMarquee;
Rect		marqueeGliderRect;
Boolean		gliderMarqueeUp;


extern	IGpCursor	*handCursor, *vertCursor, *horiCursor, *diagCursor;
extern	Rect		leftStartGliderSrc;


//==============================================================  Functions
//--------------------------------------------------------------  DoMarquee

void DoMarquee (void)
{	
	if ((!theMarquee.active) || (theMarquee.paused))
		return;

	theMarquee.step++;

	if (theMarquee.step >= theMarquee.interval)
	{
		DrawSurface *surface = mainWindow->GetDrawSurface();
		const uint8_t *pattern = theMarquee.pats[theMarquee.index];
		DrawMarquee(surface, pattern);
		theMarquee.index++;
		if (theMarquee.index >= kNumMarqueePats)
			theMarquee.index = 0;

		pattern = theMarquee.pats[theMarquee.index];
		DrawMarquee(surface, pattern);

		theMarquee.step = 0;
	}
}

//--------------------------------------------------------------  StartMarquee

void StartMarquee (Rect *theRect)
{
	if (theMarquee.active)
		StopMarquee();
	
	if (objActive == kNoObjectSelected)
		return;

	DrawSurface *surface = mainWindow->GetDrawSurface();

	theMarquee.bounds = *theRect;
	theMarquee.active = true;
	theMarquee.paused = false;
	theMarquee.handled = false;

	const uint8_t *pattern = theMarquee.pats[theMarquee.index];
	DrawMarquee(surface, pattern);
	SetCoordinateHVD(theMarquee.bounds.left, theMarquee.bounds.top, -1);
}

//--------------------------------------------------------------  StartMarqueeHandled

void StartMarqueeHandled (Rect *theRect, short direction, short dist)
{
	if (theMarquee.active)
		StopMarquee();
	
	if (objActive == kNoObjectSelected)
		return;
	
	DrawSurface *surface = mainWindow->GetDrawSurface();

	theMarquee.bounds = *theRect;
	theMarquee.active = true;
	theMarquee.paused = false;
	theMarquee.handled = true;
	QSetRect(&theMarquee.handle, 0, 0, kHandleSideLong, kHandleSideLong);
	QOffsetRect(&theMarquee.handle, kHandleSideLong / -2, kHandleSideLong / -2);
	switch (direction)
	{
		case kAbove:
		QOffsetRect(&theMarquee.handle, theMarquee.bounds.left, 
				theMarquee.bounds.top);
		QOffsetRect(&theMarquee.handle, HalfRectWide(&theMarquee.bounds), -dist);
		break;
		
		case kToRight:
		QOffsetRect(&theMarquee.handle, theMarquee.bounds.right, 
				theMarquee.bounds.top);
		QOffsetRect(&theMarquee.handle, dist, HalfRectTall(&theMarquee.bounds));
		break;
		
		case kBelow:
		QOffsetRect(&theMarquee.handle, theMarquee.bounds.left, 
				theMarquee.bounds.bottom);
		QOffsetRect(&theMarquee.handle, HalfRectWide(&theMarquee.bounds), dist);
		break;
		
		case kToLeft:
		QOffsetRect(&theMarquee.handle, theMarquee.bounds.left, 
				theMarquee.bounds.top);
		QOffsetRect(&theMarquee.handle, -dist, HalfRectTall(&theMarquee.bounds));
		break;
		
		case kBottomCorner:
		QOffsetRect(&theMarquee.handle, theMarquee.bounds.right, 
				theMarquee.bounds.bottom);
		break;
		
		case kTopCorner:
		QOffsetRect(&theMarquee.handle, theMarquee.bounds.right, 
				theMarquee.bounds.top);
		break;
	}
	theMarquee.direction = direction;
	theMarquee.dist = dist;
	
	const uint8_t *pattern = theMarquee.pats[theMarquee.index];
	DrawMarquee(surface, pattern);
	SetCoordinateHVD(theMarquee.bounds.left, theMarquee.bounds.top, dist);
}

//--------------------------------------------------------------  StopMarquee

void StopMarquee (void)
{
	if (gliderMarqueeUp)
	{
		DrawGliderMarquee();
		gliderMarqueeUp = false;
	}
	
	if (!theMarquee.active)
		return;
	
	DrawSurface *surface = mainWindow->GetDrawSurface();
	const uint8_t *pattern = theMarquee.pats[theMarquee.index];
	DrawMarquee(surface, pattern);

	theMarquee.active = false;
	SetCoordinateHVD(-1, -1, -1);
}

//--------------------------------------------------------------  PauseMarquee

void PauseMarquee (void)
{
	if (!theMarquee.active)
		return;
	
	theMarquee.paused = true;
	StopMarquee();
}

//--------------------------------------------------------------  ResumeMarquee

void ResumeMarquee (void)
{
	if (!theMarquee.paused)
		return;
	
	if (theMarquee.handled)
	{
		StartMarqueeHandled(&theMarquee.bounds, theMarquee.direction, theMarquee.dist);
		HandleBlowerGlider();
	}
	else
		StartMarquee(&theMarquee.bounds);
}

//--------------------------------------------------------------  DragOutMarqueeRect

void DragOutMarqueeRect (Window *window, Point start, Rect *theRect)
{
	Point		wasPt, newPt;
	DrawSurface	*surface = mainWindow->GetDrawSurface();
	
	InitCursor();
	QSetRect(theRect, start.h, start.v, start.h, start.v);

	const uint8_t *pattern = theMarquee.pats[theMarquee.index];
	surface->InvertFrameRect(*theRect, pattern);
	wasPt = start;
	
	while (WaitMouseUp())
	{
		GetMouse(window, &newPt);
		if (wasPt != newPt)
		{
			surface->InvertFrameRect(*theRect, pattern);
			QSetRect(theRect, start.h, start.v, newPt.h, newPt.v);
			NormalizeRect(theRect);
			surface->InvertFrameRect(*theRect, pattern);
			wasPt = newPt;
		}
	}
	surface->InvertFrameRect(*theRect, pattern);
}

//--------------------------------------------------------------  DragMarqueeRect

void DragMarqueeRect (Window *window, DrawSurface *surface, Point start, Rect *theRect, Boolean lockH, Boolean lockV)
{
	Point		wasPt, newPt;
	short		deltaH, deltaV;
	
	PLDrivers::GetDisplayDriver()->SetCursor(handCursor);
	StopMarquee();

	const uint8_t *pattern = theMarquee.pats[theMarquee.index];
	theMarquee.bounds = *theRect;
	surface->InvertFrameRect(theMarquee.bounds, pattern);
	
	wasPt = start;
	while (WaitMouseUp())
	{
		GetMouse(window, &newPt);
		if (wasPt != newPt)
		{
			if (lockV)
				deltaH = 0;
			else
				deltaH = newPt.h - wasPt.h;
			if (lockH)
				deltaV = 0;
			else
				deltaV = newPt.v - wasPt.v;
			surface->InvertFrameRect(theMarquee.bounds, pattern);
			QOffsetRect(&theMarquee.bounds, deltaH, deltaV);
			surface->InvertFrameRect(theMarquee.bounds, pattern);
			wasPt = newPt;
			SetCoordinateHVD(theMarquee.bounds.left, theMarquee.bounds.top, -2);
		}
	}
	surface->InvertFrameRect(theMarquee.bounds, pattern);

	*theRect = theMarquee.bounds;

	InitCursor();
}

//--------------------------------------------------------------  DragMarqueeHandle

void DragMarqueeHandle (Window *window, DrawSurface *surface, Point start, short *dragged)
{
	Point		wasPt, newPt;
	short		deltaH, deltaV;
	
	if ((theMarquee.direction == kAbove) || (theMarquee.direction == kBelow))
		PLDrivers::GetDisplayDriver()->SetCursor(vertCursor);
	else
		PLDrivers::GetDisplayDriver()->SetCursor(horiCursor);
	StopMarquee();

	const uint8_t *pattern = theMarquee.pats[theMarquee.index];
	surface->InvertFrameRect(theMarquee.bounds, pattern);
	surface->InvertFillRect(theMarquee.handle, pattern);
	
	wasPt = start;
	while (WaitMouseUp())
	{
		GetMouse(window, &newPt);
		if (wasPt != newPt)
		{
			switch (theMarquee.direction)
			{
				case kAbove:
				deltaH = 0;
				deltaV = newPt.v - wasPt.v;
				*dragged -= deltaV;
				if (*dragged <= 0)
				{
					deltaV += *dragged;
					*dragged = 0;
				}
				DeltaCoordinateD(*dragged);
				break;
				
				case kToRight:
				deltaH = newPt.h - wasPt.h;
				deltaV = 0;
				*dragged += deltaH;
				if (*dragged <= 0)
				{
					deltaH -= *dragged;
					*dragged = 0;
				}
				DeltaCoordinateD(*dragged);
				break;
				
				case kBelow:
				deltaH = 0;
				deltaV = newPt.v - wasPt.v;
				*dragged += deltaV;
				if (*dragged <= 0)
				{
					deltaV -= *dragged;
					*dragged = 0;
				}
				DeltaCoordinateD(*dragged);
				break;
				
				case kToLeft:
				deltaH = newPt.h - wasPt.h;
				deltaV = 0;
				*dragged -= deltaH;
				if (*dragged <= 0)
				{
					deltaH += *dragged;
					*dragged = 0;
				}
				DeltaCoordinateD(*dragged);
				break;
			}

			surface->InvertFillRect(theMarquee.handle, pattern);
			QOffsetRect(&theMarquee.handle, deltaH, deltaV);
			surface->InvertFillRect(theMarquee.handle, pattern);
			wasPt = newPt;
		}
	}
	surface->InvertFrameRect(theMarquee.bounds, pattern);
	surface->InvertFillRect(theMarquee.handle, pattern);
	InitCursor();
}

//--------------------------------------------------------------  DragMarqueeCorner

void DragMarqueeCorner (Window *window, DrawSurface *surface, Point start, short *hDragged, short *vDragged, Boolean isTop)
{
	Point		wasPt, newPt;
	short		deltaH, deltaV;
	
	PLDrivers::GetDisplayDriver()->SetCursor(diagCursor);
	StopMarquee();

	const uint8_t *pattern = theMarquee.pats[theMarquee.index];
	surface->InvertFrameRect(theMarquee.bounds, pattern);
	surface->InvertFillRect(theMarquee.handle, pattern);
	
	wasPt = start;
	while (WaitMouseUp())
	{
		GetMouse(window, &newPt);
		if (wasPt != newPt)
		{
			deltaH = newPt.h - wasPt.h;
			if (isTop)
				deltaV = wasPt.v - newPt.v;
			else
				deltaV = newPt.v - wasPt.v;
			*hDragged += deltaH;
			if (*hDragged <= 0)
			{
				deltaH -= *hDragged;
				*hDragged = 0;
			}
			*vDragged += deltaV;
			if (*vDragged <= 0)
			{
				deltaV -= *vDragged;
				*vDragged = 0;
			}
			surface->InvertFrameRect(theMarquee.bounds, pattern);
			surface->InvertFillRect(theMarquee.handle, pattern);
			if (isTop)
			{
				QOffsetRect(&theMarquee.handle, deltaH, -deltaV);
				theMarquee.bounds.right += deltaH;
				theMarquee.bounds.top -= deltaV;
			}
			else
			{
				QOffsetRect(&theMarquee.handle, deltaH, deltaV);
				theMarquee.bounds.right += deltaH;
				theMarquee.bounds.bottom += deltaV;
			}
			surface->InvertFrameRect(theMarquee.bounds, pattern);
			surface->InvertFillRect(theMarquee.handle, pattern);
			wasPt = newPt;
		}
	}
	surface->InvertFrameRect(theMarquee.bounds, pattern);
	surface->InvertFillRect(theMarquee.handle, pattern);
	InitCursor();
}

//--------------------------------------------------------------  MarqueeHasHandles

Boolean MarqueeHasHandles (short *direction, short *dist)
{
	if (theMarquee.handled)
	{
		*direction = theMarquee.direction;
		*dist = theMarquee.dist;
		return (true);
	}
	else
	{
		*direction = 0;
		*dist = 0;
		return (false);
	}
}

//--------------------------------------------------------------  PtInMarqueeHandle

Boolean PtInMarqueeHandle (Point where)
{
	return theMarquee.handle.Contains(where);
}

//--------------------------------------------------------------  DrawGliderMarquee

void DrawGliderMarquee (void)
{
	DrawSurface *surface = mainWindow->GetDrawSurface();
	ImageInvert(*GetGWorldPixMap(blowerMaskMap), GetPortBitMapForCopyBits(surface), leftStartGliderSrc, marqueeGliderRect);
	surface->m_port.SetDirty(PortabilityLayer::QDPortDirtyFlag_Contents);
}

//--------------------------------------------------------------  SetMarqueeGliderCenter

void SetMarqueeGliderRect (short h, short v)
{
	marqueeGliderRect = leftStartGliderSrc;
	ZeroRectCorner(&marqueeGliderRect);
	QOffsetRect(&marqueeGliderRect, h - kHalfGliderWide, v - kGliderHigh);
	
	DrawGliderMarquee();
	gliderMarqueeUp = true;
}

//--------------------------------------------------------------  DrawMarquee

void DrawMarquee (DrawSurface *surface, const uint8_t *pattern)
{
	surface->InvertFrameRect(theMarquee.bounds, pattern);
	if (theMarquee.handled)
	{
		surface->InvertFillRect(theMarquee.handle, pattern);
		Point points[2] = { Point::Create(0, 0), Point::Create(0, 0) };

		switch (theMarquee.direction)
		{
			case kAbove:
			points[0] = Point::Create(theMarquee.handle.left + (kHandleSideLong / 2), 
					theMarquee.handle.bottom);
			points[1] = Point::Create(theMarquee.handle.left + (kHandleSideLong / 2),
					theMarquee.bounds.top - 1);
			break;
			
			case kToRight:
			points[0] = Point::Create(theMarquee.handle.left, 
					theMarquee.handle.top + (kHandleSideLong / 2));
			points[1] = Point::Create(theMarquee.bounds.right,
					theMarquee.handle.top + (kHandleSideLong / 2));
			break;
			
			case kBelow:
			points[0] = Point::Create(theMarquee.handle.left + (kHandleSideLong / 2),
					theMarquee.handle.top - 1);
			points[1] = Point::Create(theMarquee.handle.left + (kHandleSideLong / 2),
					theMarquee.bounds.bottom);
			break;
			
			case kToLeft:
			points[1] = Point::Create(theMarquee.handle.right,
					theMarquee.handle.top + (kHandleSideLong / 2));
			points[0] = Point::Create(theMarquee.bounds.left,
					theMarquee.handle.top + (kHandleSideLong / 2));
			break;
		}

		if (points[1].h < points[0].h)
			std::swap(points[0].h, points[1].h);
		if (points[1].v < points[0].v)
			std::swap(points[0].v, points[1].v);

		surface->InvertFillRect(Rect::Create(points[0].v, points[0].h, points[1].v + 1, points[1].h + 1), pattern);
	}
	
	if (gliderMarqueeUp)
		DrawGliderMarquee();
}

//--------------------------------------------------------------  InitMarquee

void InitMarquee (void)
{
	short		i;
	
	for (i = 0; i < kNumMarqueePats; i++)
		GetIndPattern(&theMarquee.pats[i], kMarqueePatListID, i + 1);
	theMarquee.index = 0;
	theMarquee.active = false;
	theMarquee.paused = false;
	theMarquee.handled = false;
	theMarquee.step = 0;
	theMarquee.interval = 2;
	gliderMarqueeUp = false;
}

