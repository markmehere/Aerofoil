#include "PLAppleEvents.h"
#include "AEManager.h"

OSErr AEGetParamDesc(const AppleEvent *evt, AEKeyword keyword, DescType desiredType, AEDescList *descList)
{
	PL_NotYetImplemented();
	return noErr;
}

OSErr AEDisposeDesc(AEDescList *descList)
{
	PL_NotYetImplemented();
	return noErr;
}

OSErr AECountItems(AEDescList *descList, long *count)
{
	PL_NotYetImplemented();
	return noErr;
}

OSErr AEGetNthPtr(AEDescList *descList, long index, DescType desiredType, AEKeyword *keyword, DescType *type, void *data, Size maxSize, Size *actualSize)
{
	PL_NotYetImplemented();
	return noErr;
}

OSErr AEGetAttributePtr(const AppleEvent *evt, AEKeyword keyword, DescType desiredType, DescType *type, void *data, Size maxSize, Size *actualSize)
{
	PL_NotYetImplemented();
	return noErr;
}

OSErr AEInstallEventHandler(AEEventClass eventClass, AEEventID eventID, AEEventHandlerUPP handler, UInt32 ref, bool isSysHandler)
{
	PortabilityLayer::AEManager::GetInstance()->InstallEventHandler(eventClass, eventID, handler, ref, isSysHandler);
	return noErr;
}

OSErr AESetInteractionAllowed(AEInteractAllowed level)
{
	PortabilityLayer::AEManager::GetInstance()->SetInteractAllowed(level);
	return noErr;
}

AEEventHandlerUPP NewAEEventHandlerProc(AEEventHandler handler)
{
	return handler;
}
