#pragma once
#ifndef __PL_NAVIGATION_H__
#define __PL_NAVIGATION_H__

#include "PLCore.h"
#include "PLAppleEvents.h"

struct AEDesc;

struct NavReplyRecord
{
	bool validRecord;
	bool replacing;
	int vRefNum;
	long parID;	// Directory?
	AEDescList selection;
};

struct NavDialogOptions
{
};

PLError_t NavGetDefaultDialogOptions(NavDialogOptions *options);
PLError_t NavPutFile(AEDesc *defaultLocation, NavReplyRecord *reply, NavDialogOptions *dlgOptions, void *unknown, UInt32 fileType, UInt32 fileCreator, void *unknown2);

#endif
