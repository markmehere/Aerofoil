
//============================================================================
//----------------------------------------------------------------------------
//								  AppleEvents.c
//----------------------------------------------------------------------------
//============================================================================


#include "AppEventHandler.h"
#include "DialogManager.h"
#include "Externs.h"
#include "House.h"


#define kNoPrintingAlert			1031

extern	Boolean		quitting;

#if 0

PLError_t DoOpenAppAE (const AppleEvent *, AppleEvent *, UInt32);
PLError_t DoOpenDocAE (const AppleEvent *, AppleEvent *, UInt32);
PLError_t DoPrintDocAE (const AppleEvent *, AppleEvent *, UInt32);
PLError_t DoQuitAE (const AppleEvent *, AppleEvent *, UInt32);
PLError_t MyGotRequiredParams (const AppleEvent *);

AEEventHandlerUPP	openAppAEUPP, openDocAEUPP, printDocAEUPP, quitAEUPP;


extern	VFileSpec	*theHousesSpecs;
extern	long		incrementModeTime;
extern	short		thisHouseIndex, splashOriginH, splashOriginV;


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
	size_t			actualSize;
	AEKeyword		keywd;
	DescType		returnedType;
	PLError_t			theErr, whoCares;
	short			i;
	bool			redrawMainWindow = false;
	
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
					redrawMainWindow = true;
				}
			}
		}
		InitCursor();
	}
#endif
	whoCares = AEDisposeDesc(&docList);

	if (redrawMainWindow)
		UpdateMainWindow();
	
	return theErr;
}

//--------------------------------------------------------------  DoPrintDocAE
// Handles a "Print Document" Apple Event.

PLError_t DoPrintDocAE (const AppleEvent *theAE, AppleEvent *reply, UInt32 ref)
{
	short		hitWhat;
	
//	CenterAlert(kNoPrintingAlert);
	hitWhat = PortabilityLayer::DialogManager::GetInstance()->DisplayAlert(kNoPrintingAlert, nullptr);
	
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
	size_t			actualSize;
	
	return (AEGetAttributePtr(theAE, keyMissedKeywordAttr, typeWildCard,
			&returnedType, 0L, 0, &actualSize) == errAEDescNotFound) ? PLErrors::kNone :
			PLErrors::kInvalidParameter;
}
#endif

class SystemEventHandlerImpl : public PortabilityLayer::IAppEventHandler
{
public:
	void OnQuit() override;

	static SystemEventHandlerImpl *GetInstance();

private:
	static SystemEventHandlerImpl ms_instance;
};

void SystemEventHandlerImpl::OnQuit()
{
	quitting = true;
}


SystemEventHandlerImpl *SystemEventHandlerImpl::GetInstance()
{
	return &ms_instance;
}

SystemEventHandlerImpl SystemEventHandlerImpl::ms_instance;

//--------------------------------------------------------------  SetUpAppleEvents
// Initializes all handlers, etc. for dealing with Apple Events.

void SetUpAppleEvents (void)
{
	PLError_t		theErr;

	PortabilityLayer::AppEventHandler::SetInstance(SystemEventHandlerImpl::GetInstance());
}

