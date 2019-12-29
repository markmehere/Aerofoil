
//============================================================================
//----------------------------------------------------------------------------
//								  AppleEvents.c
//----------------------------------------------------------------------------
//============================================================================


#include "PLAppleEvents.h"
#include "Externs.h"
#include "House.h"


#define kNoPrintingAlert			1031


PLError_t DoOpenAppAE (const AppleEvent *, AppleEvent *, UInt32);
PLError_t DoOpenDocAE (const AppleEvent *, AppleEvent *, UInt32);
PLError_t DoPrintDocAE (const AppleEvent *, AppleEvent *, UInt32);
PLError_t DoQuitAE (const AppleEvent *, AppleEvent *, UInt32);
PLError_t MyGotRequiredParams (const AppleEvent *);

AEEventHandlerUPP	openAppAEUPP, openDocAEUPP, printDocAEUPP, quitAEUPP;


extern	VFileSpec	*theHousesSpecs;
extern	long		incrementModeTime;
extern	short		thisHouseIndex, splashOriginH, splashOriginV;
extern	Boolean		quitting;


//==============================================================  Functions
//--------------------------------------------------------------  DoOpenAppAE
// Handles an "Open Application" Apple Event.

PLError_t DoOpenAppAE (const AppleEvent *theAE, AppleEvent *reply, UInt32 ref)
{
	PLError_t		theErr;
	
	theErr = MyGotRequiredParams(theAE);
	return (theErr);
}

//--------------------------------------------------------------  DoOpenDocAE
// Handles an "Open Document" Apple Event.

PLError_t DoOpenDocAE (const AppleEvent *theAE, AppleEvent *reply, UInt32 ref)
{
	VFileSpec		oneFSS;
	VFileInfo		finderInfo;
	AEDescList		docList;
	long			itemsInList;
	Size			actualSize;
	AEKeyword		keywd;
	DescType		returnedType;
	PLError_t			theErr, whoCares;
	short			i;
	
	theErr = AEGetParamDesc(theAE, keyDirectObject, typeAEList, &docList);
	if (theErr != PLErrors::kNone)
	{
		YellowAlert(kYellowAppleEventErr, theErr);
		return (theErr);
	}
	
	theErr = MyGotRequiredParams(theAE);
	if (theErr != PLErrors::kNone)
	{
		whoCares = AEDisposeDesc(&docList);
		return (theErr);
	}
	
	theErr = AECountItems(&docList, &itemsInList);
	if (theErr != PLErrors::kNone)
	{
		whoCares = AEDisposeDesc(&docList);
		return (theErr);
	}
	
#ifndef COMPILEDEMO	
	for (i = 1; i <= itemsInList; i++)
	{
		theErr = AEGetNthPtr(&docList, i, typeFSS, &keywd, &returnedType,
				&oneFSS, sizeof(oneFSS), &actualSize);
		if (theErr == PLErrors::kNone)
		{
			theErr = FSpGetFInfo(oneFSS, finderInfo);
			if ((theErr == PLErrors::kNone) && (finderInfo.m_type == 'gliH'))
				AddExtraHouse(oneFSS);
		}
	}
	if (itemsInList > 0)
	{
		theErr = AEGetNthPtr(&docList, 1, typeFSS, &keywd, &returnedType,
				&oneFSS, sizeof(oneFSS), &actualSize);
		if (theErr == PLErrors::kNone)
		{
			theErr = FSpGetFInfo(oneFSS, finderInfo);
			if ((theErr == PLErrors::kNone) && (finderInfo.m_type == 'gliH'))
			{
				CloseHouse();
				PasStringCopy(oneFSS.m_name, thisHouseName);
				BuildHouseList();
				if (OpenHouse())
					ReadHouse();
				PasStringCopy(theHousesSpecs[thisHouseIndex].m_name, thisHouseName);
				OpenCloseEditWindows();
				incrementModeTime = TickCount() + kIdleSplashTicks;
				if ((theMode == kSplashMode) || (theMode == kPlayMode))
				{
					Rect		updateRect;
					
					SetRect(&updateRect, splashOriginH + 474, splashOriginV + 304, splashOriginH + 474 + 166, splashOriginV + 304 + 12);
					InvalWindowRect(mainWindow, &updateRect);
				}
			}
		}
		InitCursor();
	}
#endif
	whoCares = AEDisposeDesc(&docList);
	
	return theErr;
}

//--------------------------------------------------------------  DoPrintDocAE
// Handles a "Print Document" Apple Event.

PLError_t DoPrintDocAE (const AppleEvent *theAE, AppleEvent *reply, UInt32 ref)
{
	short		hitWhat;
	
//	CenterAlert(kNoPrintingAlert);
	hitWhat = Alert(kNoPrintingAlert, nil);
	
	return PLErrors::kInvalidParameter;
}

//--------------------------------------------------------------  DoQuitAE
// Handles a "Quit Application" Apple Event.

PLError_t DoQuitAE (const AppleEvent *theAE, AppleEvent *reply, UInt32 ref)
{
	PLError_t			isHuman;
	
	isHuman = MyGotRequiredParams(theAE);
	if (isHuman == PLErrors::kNone)
		quitting = true;
	
	return isHuman;
}

//--------------------------------------------------------------  MyGotRequiredParams
// Have no clue!  :)

PLError_t MyGotRequiredParams (const AppleEvent *theAE)
{
	DescType		returnedType;
	Size			actualSize;
	
	return (AEGetAttributePtr(theAE, keyMissedKeywordAttr, typeWildCard,
			&returnedType, 0L, 0, &actualSize) == errAEDescNotFound) ? PLErrors::kNone :
			PLErrors::kInvalidParameter;
}

//--------------------------------------------------------------  SetUpAppleEvents
// Initializes all handlers, etc. for dealing with Apple Events.

void SetUpAppleEvents (void)
{
	PLError_t		theErr;
	
	openAppAEUPP = NewAEEventHandlerProc(DoOpenAppAE);
	openDocAEUPP = NewAEEventHandlerProc(DoOpenDocAE);
	printDocAEUPP = NewAEEventHandlerProc(DoPrintDocAE);
	quitAEUPP = NewAEEventHandlerProc(DoQuitAE);
	
	theErr = AEInstallEventHandler(kCoreEventClass,		// install oapp 
			kAEOpenApplication, openAppAEUPP, 0, false);
	if (theErr != PLErrors::kNone)
		YellowAlert(kYellowAppleEventErr, theErr);
	
	theErr = AEInstallEventHandler(kCoreEventClass, 	// install odoc 
			kAEOpenDocuments, openDocAEUPP, 0, false);
	if (theErr != PLErrors::kNone)
		YellowAlert(kYellowAppleEventErr, theErr);
	
	theErr = AEInstallEventHandler(kCoreEventClass, 	// install pdoc 
			kAEPrintDocuments, printDocAEUPP, 0, false);
	if (theErr != PLErrors::kNone)
		YellowAlert(kYellowAppleEventErr, theErr);
	
	theErr = AEInstallEventHandler(kCoreEventClass, 	// install quit 
			kAEQuitApplication, quitAEUPP, 0, false);
	if (theErr != PLErrors::kNone)
		YellowAlert(kYellowAppleEventErr, theErr);
	
	theErr = AESetInteractionAllowed(kAEInteractWithAll);
	if (theErr != PLErrors::kNone)
		YellowAlert(kYellowAppleEventErr, theErr);
}

