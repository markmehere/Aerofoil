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

typedef Picture *PicPtr;
typedef PicPtr *PicHandle;

enum QDFlags
{
	useTempMem = 1,
};

OSErr NewGWorld(GWorldPtr *gworld, int depth, const Rect *bounds, CTabHandle colorTable, int flags);
void DisposeGWorld(GWorldPtr gworld);

PixMapHandle GetGWorldPixMap(GWorldPtr gworld);
void LockPixels(PixMapHandle pixmap);

PicHandle GetPicture(short resID);

void OffsetRect(Rect *rect, int right, int down);

void DrawPicture(PicHandle pict, Rect *bounds);

CGrafPtr GetGraphicsPort();
void SetGraphicsPort(CGrafPtr gw);


#endif
