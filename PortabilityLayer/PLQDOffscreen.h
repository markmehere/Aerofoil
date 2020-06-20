#pragma once
#ifndef __PL_QDOFFSCREEN_H__
#define __PL_QDOFFSCREEN_H__

#include "PLCore.h"
#include "PLQDraw.h"

struct ColorTable
{
};

struct PixMap;

typedef ColorTable *CTabPtr;
typedef CTabPtr *CTabHandle;

typedef PixMap *PixMapPtr;
typedef PixMapPtr *PixMapHandle;

PLError_t NewGWorld(DrawSurface **gworld, GpPixelFormat_t pixelFormat, const Rect *bounds, CTabHandle colorTable);
void DisposeGWorld(DrawSurface *gworld);

PixMapHandle GetGWorldPixMap(DrawSurface *gworld);

void OffsetRect(Rect *rect, int right, int down);


#endif
