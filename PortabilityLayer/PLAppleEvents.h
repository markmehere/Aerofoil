#pragma once
#ifndef __PL_APPLEEVENTS_H__
#define __PL_APPLEEVENTS_H__

#include "PLCore.h"
#include "PLAppleEventsCommonTypes.h"

struct AppleEvent
{
};

struct AEDescList
{
};

struct AEDesc
{
};

typedef AEEventHandler AEEventHandlerUPP;

OSErr AEGetParamDesc(const AppleEvent *evt, AEKeyword keyword, DescType desiredType, AEDescList *descList);
OSErr AEDisposeDesc(AEDescList *descList);
OSErr AECountItems(AEDescList *descList, long *count);
OSErr AEGetNthPtr(AEDescList *descList, long index, DescType desiredType, AEKeyword *keyword, DescType *type, void *data, Size maxSize, Size *actualSize);
OSErr AEGetAttributePtr(const AppleEvent *evt, AEKeyword keyword, DescType desiredType, DescType *type, void *data, Size maxSize, Size *actualSize);
OSErr AEInstallEventHandler(AEEventClass eventClass, AEEventID eventID, AEEventHandlerUPP handler, UInt32 ref, bool isSysHandler);
OSErr AESetInteractionAllowed(AEInteractAllowed level);

AEEventHandlerUPP NewAEEventHandlerProc(AEEventHandler handler);

#endif
