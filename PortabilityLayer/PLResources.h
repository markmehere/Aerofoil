#pragma once
#ifndef __PL_RESOURCES_H__
#define __PL_RESOURCES_H__

#include "PLCore.h"


class PLPasStr;

short CurResFile();
void UseResFile(short fid);
Handle Get1Resource(UInt32 resID, int index);
Handle Get1IndResource(UInt32 resID, int index);
int Count1Resources(UInt32 resType);

void SetResLoad(Boolean load);	// Sets whether resources should be loaded when requested

long GetMaxResourceSize(Handle res);

// This should return -1 on error?
short HOpenResFile(PortabilityLayer::VirtualDirectory_t dirID, const PLPasStr &name, int permissions);


#endif
