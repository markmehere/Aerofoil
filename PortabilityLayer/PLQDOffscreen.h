#pragma once
#ifndef __PL_QDOFFSCREEN_H__
#define __PL_QDOFFSCREEN_H__

#include "PLCore.h"
#include "PLQuickdraw.h"

struct ColorTable
{
};

struct PixMap
{
};

struct Picture
{
	Rect picFrame;
};

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

OSErr NewGWorld(GWorldPtr *gworld, int depth, Rect *bounds, CTabHandle colorTable, GDHandle device, int flags);
void DisposeGWorld(GWorldPtr gworld);

PixMapHandle GetGWorldPixMap(GWorldPtr gworld);
void LockPixels(PixMapHandle pixmap);

PicHandle GetPicture(short resID);

void OffsetRect(Rect *rect, int right, int down);

void DrawPicture(PicHandle pict, Rect *bounds);

void GetGWorld(CGrafPtr *gw, GDHandle *gdHandle);
void SetGWorld(CGrafPtr gw, GDHandle gdHandle);


#endif
