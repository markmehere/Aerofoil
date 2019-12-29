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

PLError_t AEGetParamDesc(const AppleEvent *evt, AEKeyword keyword, DescType desiredType, AEDescList *descList);
PLError_t AEDisposeDesc(AEDescList *descList);
PLError_t AECountItems(AEDescList *descList, long *count);
PLError_t AEGetNthPtr(AEDescList *descList, long index, DescType desiredType, AEKeyword *keyword, DescType *type, void *data, Size maxSize, Size *actualSize);
PLError_t AEGetAttributePtr(const AppleEvent *evt, AEKeyword keyword, DescType desiredType, DescType *type, void *data, Size maxSize, Size *actualSize);
PLError_t AEInstallEventHandler(AEEventClass eventClass, AEEventID eventID, AEEventHandlerUPP handler, UInt32 ref, bool isSysHandler);
PLError_t AESetInteractionAllowed(AEInteractAllowed level);

AEEventHandlerUPP NewAEEventHandlerProc(AEEventHandler handler);

#endif
