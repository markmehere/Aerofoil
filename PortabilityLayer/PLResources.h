#pragma once
#ifndef __PL_RESOURCES_H__
#define __PL_RESOURCES_H__

#include "PLCore.h"

struct ResType
{
};

class PLPasStr;

void DetachResource(Handle hdl);
void ReleaseResource(Handle hdl);

short CurResFile();
void UseResFile(short fid);
Handle Get1Resource(UInt32 resID, int index);
Handle Get1IndResource(UInt32 resID, int index);
int Count1Resources(UInt32 resType);

void HCreateResFile(int refNum, long dirID, const PLPasStr &name);
OSErr ResError();

short FSpOpenResFile(const FSSpec *spec, int permission);
void CloseResFile(short refNum);

void SetResLoad(Boolean load);	// Sets whether resources should be loaded when requested

long GetMaxResourceSize(Handle res);
void GetResInfo(Handle res, short *resID, ResType *resType, Str255 resName);

// This should return -1 on error?
short HOpenResFile(short refNum, long parID, const PLPasStr &name, int permissions);


#endif
