
//============================================================================
//----------------------------------------------------------------------------
//								   FileError.c
//----------------------------------------------------------------------------
//============================================================================


#include "PLNumberFormatting.h"
#include "PLTextUtils.h"
#include "PLPasStr.h"
#include "DialogManager.h"
#include "Externs.h"


#define	rFileErrorAlert		140
#define	rFileErrorStrings	140


//==============================================================  Functions
//--------------------------------------------------------------  CheckFileError

// Given a result code (returned from a previous file operation) thisÉ
// function cheks to see if the result code is an error and, if it isÉ
// a common error for which I have a string message, I bring up anÉ
// alert with the error message.  If it is an unusual error, I stillÉ
// bring up an alert but with "Miscellaneous file error" and theÉ
// error ID.

Boolean CheckFileError (short resultCode, const PLPasStr &fileName)
{
	short			dummyInt, stringIndex;
	Str255			errMessage, errNumString;
	
	if (resultCode == PLErrors::kNone)		// No problems?  Then cruise
		return(true);
	
	switch (resultCode)
	{
		case PLErrors::kIOError:
			stringIndex = 4;
			break;
		case PLErrors::kBadFileName:
			stringIndex = 5;
			break;
		case PLErrors::kAccessDenied:
			stringIndex = 6;
			break;
		case PLErrors::kOutOfMemory:
			stringIndex = 7;
			break;
		default:
			stringIndex = 1;
			break;
	}
	InitCursor();
	
	GetIndString(errMessage, rFileErrorStrings, stringIndex);
	NumToString((long)resultCode, errNumString);
	ParamText(errMessage, errNumString, fileName, PSTR(""));
	
//	CenterAlert(rFileErrorAlert);
	dummyInt = PortabilityLayer::DialogManager::GetInstance()->DisplayAlert(rFileErrorAlert);
	
	return(false);
}
