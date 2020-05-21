//============================================================================
//----------------------------------------------------------------------------
//								   RectUtils.h
//----------------------------------------------------------------------------
//============================================================================


#pragma once


#include "PLQDraw.h"

namespace PortabilityLayer
{
	class ResolveCachingColor;
}


void FrameWHRect (short, short, short, short, PortabilityLayer::ResolveCachingColor &color);
void NormalizeRect (Rect *);
void ZeroRectCorner (Rect *);
void CenterRectOnPoint (Rect *, Point);
short HalfRectWide (Rect *);
short HalfRectTall (Rect *);
short RectWide (Rect *);
short RectTall (Rect *);
void CenterRectInRect (Rect *, Rect *);
void HOffsetRect (Rect *, short);
void VOffsetRect (Rect *, short);
Boolean IsRectLeftOfRect (Rect *, Rect *);
void QOffsetRect (Rect *, short, short);
void QSetRect (Rect *, short, short, short, short);
Boolean ForceRectInRect (Rect *, Rect *);
void QUnionSimilarRect (Rect *, Rect *, Rect *);
void SetEraseRect (short, short, short, short);
