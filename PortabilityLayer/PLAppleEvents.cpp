#include "PLAppleEvents.h"
#include "AEManager.h"

PLError_t AEGetParamDesc(const AppleEvent *evt, AEKeyword keyword, DescType desiredType, AEDescList *descList)
{
	PL_NotYetImplemented();
	return PLErrors::kNone;
}

PLError_t AEDisposeDesc(AEDescList *descList)
{
	PL_NotYetImplemented();
	return PLErrors::kNone;
}

PLError_t AECountItems(AEDescList *descList, long *count)
{
	PL_NotYetImplemented();
	return PLErrors::kNone;
}

PLError_t AEGetNthPtr(AEDescList *descList, long index, DescType desiredType, AEKeyword *keyword, DescType *type, void *data, size_t maxSize, size_t *actualSize)
{
	PL_NotYetImplemented();
	return PLErrors::kNone;
}

PLError_t AEGetAttributePtr(const AppleEvent *evt, AEKeyword keyword, DescType desiredType, DescType *type, void *data, size_t maxSize, size_t *actualSize)
{
	PL_NotYetImplemented();
	return PLErrors::kNone;
}

PLError_t AEInstallEventHandler(AEEventClass eventClass, AEEventID eventID, AEEventHandlerUPP handler, UInt32 ref, bool isSysHandler)
{
	PortabilityLayer::AEManager::GetInstance()->InstallEventHandler(eventClass, eventID, handler, ref, isSysHandler);
	return PLErrors::kNone;
}

PLError_t AESetInteractionAllowed(AEInteractAllowed level)
{
	PortabilityLayer::AEManager::GetInstance()->SetInteractAllowed(level);
	return PLErrors::kNone;
}

AEEventHandlerUPP NewAEEventHandlerProc(AEEventHandler handler)
{
	return handler;
}
