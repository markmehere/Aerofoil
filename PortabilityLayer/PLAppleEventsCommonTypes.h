#pragma once
#ifndef __PL_APPLE_EVENTS_COMMON_TYPES_H__
#define __PL_APPLE_EVENTS_COMMON_TYPES_H__

#include "PLErrorCodes.h"

#include <stdint.h>

struct AppleEvent;

enum AEError
{
	errAEEventNotHandled = 1,
	errAEDescNotFound,
	errAEParamMissed,
};

enum AEKeyword
{
	keyDirectObject,
	keyMissedKeywordAttr,
};

enum DescType
{
	typeAEList,
	typeFSS,
	typeWildCard,
};

enum AEEventID
{
	kAEOpenApplication,
	kAEOpenDocuments,
	kAEPrintDocuments,
	kAEQuitApplication
};

enum AEEventClass
{
	kCoreEventClass
};

enum AEInteractAllowed
{
	kAEInteractUnknown,
	kAEInteractWithAll
};

typedef PLError_t (*AEEventHandler)(const AppleEvent *theAE, AppleEvent *reply, uint32_t ref);

#endif
