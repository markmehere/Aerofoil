
//============================================================================
//----------------------------------------------------------------------------
//									Prefs.c
//----------------------------------------------------------------------------
//============================================================================

#include "PLPasStr.h"
#include "Externs.h"
#include "Environ.h"


#define	kPrefCreatorType	'ozm5'
#define	kPrefFileType		'gliP'
#define	kPrefFileName		PSTR("Glider Prefs")
#define	kDefaultPrefFName	PSTR("Preferences")
#define kPrefsStringsID		160
#define kNewPrefsAlertID	160
#define	kPrefsFNameIndex	1


Boolean CanUseFindFolder (void);
Boolean GetPrefsFPath (long *, short *);
Boolean CreatePrefsFolder (short *);
Boolean WritePrefs (long *, short *, prefsInfo *);
OSErr ReadPrefs (long *, short *, prefsInfo *);
Boolean DeletePrefs (long *, short *);
void BringUpDeletePrefsAlert (void);


//==============================================================  Functions
//--------------------------------------------------------------  CanUseFindFolder

Boolean CanUseFindFolder (void)
{
	return true;
}

//--------------------------------------------------------------  GetPrefsFPath

Boolean GetPrefsFPath (long *prefDirID, short *systemVolRef)
{
	OSErr		theErr;
	
	theErr = FindFolder(kOnSystemDisk, kPreferencesFolderType, kCreateFolder, 
		systemVolRef, prefDirID);
	if (theErr != noErr)
		return(false);
	
	return(true);
}

//--------------------------------------------------------------  CreatePrefsFolder

Boolean CreatePrefsFolder (short *systemVolRef)
{
	HFileParam	fileParamBlock;
	Str255		folderName;
	OSErr		theErr;
	
	GetIndString(folderName, kPrefsStringsID, kPrefsFNameIndex);
	
	fileParamBlock.ioVRefNum = *systemVolRef;
	fileParamBlock.ioDirID = 0;
	fileParamBlock.ioNamePtr = folderName;
	fileParamBlock.ioCompletion = nil;
	
	theErr = PBDirCreate(&fileParamBlock, false);
	if (theErr != noErr)
	{
		CheckFileError(theErr, PSTR("Preferences"));
		return(false);
	}
	return(true);
}

//--------------------------------------------------------------  WritePrefs

Boolean WritePrefs (long *prefDirID, short *systemVolRef, prefsInfo *thePrefs)
{
	OSErr		theErr;
	short		fileRefNum;
	long		byteCount;
	FSSpec		theSpecs;
	Str255		fileName;
	
	PasStringCopy(kPrefFileName, fileName);
	
	theErr = FSMakeFSSpec(*systemVolRef, *prefDirID, fileName, &theSpecs);
	if (theErr != noErr)
	{
		if (theErr != fnfErr)
		{
			CheckFileError(theErr, PSTR("Preferences"));
			return(false);
		}
		theErr = FSpCreate(&theSpecs, kPrefCreatorType, kPrefFileType, smSystemScript);
		if (theErr != noErr)
		{
			CheckFileError(theErr, PSTR("Preferences"));
			return(false);
		}
	}
	theErr = FSpOpenDF(&theSpecs, fsRdWrPerm, &fileRefNum);
	if (theErr != noErr)
	{
		CheckFileError(theErr, PSTR("Preferences"));
		return(false);
	}
	
	byteCount = sizeof(*thePrefs);
	
	theErr = FSWrite(fileRefNum, &byteCount, thePrefs);
	if (theErr != noErr)
	{
		CheckFileError(theErr, PSTR("Preferences"));
		return(false);
	}
	
	theErr = FSClose(fileRefNum);
	if (theErr != noErr)
	{
		CheckFileError(theErr, PSTR("Preferences"));
		return(false);
	}
	
	return(true);
}

//--------------------------------------------------------------  SavePrefs

Boolean SavePrefs (prefsInfo *thePrefs, short versionNow)
{
	long		prefDirID;
	short		systemVolRef;
	
	thePrefs->prefVersion = versionNow;
	
	if (!GetPrefsFPath(&prefDirID, &systemVolRef))
		return(false);
	
	if (!WritePrefs(&prefDirID, &systemVolRef, thePrefs))
		return(false);
	
	return(true);
}

//--------------------------------------------------------------  ReadPrefs

OSErr ReadPrefs (long *prefDirID, short *systemVolRef, prefsInfo *thePrefs)
{
	OSErr		theErr;
	short		fileRefNum;
	long		byteCount;
	FSSpec		theSpecs;
	Str255		fileName;
	
	PasStringCopy(kPrefFileName, fileName);
	
	theErr = FSMakeFSSpec(*systemVolRef, *prefDirID, fileName, &theSpecs);
	if (theErr != noErr)
	{
		if (theErr == fnfErr)
			return(theErr);
		else
		{
			CheckFileError(theErr, PSTR("Preferences"));
			return(theErr);
		}
	}
	
	theErr = FSpOpenDF(&theSpecs, fsRdWrPerm, &fileRefNum);
	if (theErr != noErr)
	{
		CheckFileError(theErr, PSTR("Preferences"));
		return(theErr);
	}
	
	byteCount = sizeof(*thePrefs);
	
	theErr = FSRead(fileRefNum, &byteCount, thePrefs);
	if (theErr != noErr)
	{
		if (theErr == eofErr)
			theErr = FSClose(fileRefNum);
		else
		{
			CheckFileError(theErr, PSTR("Preferences"));
			theErr = FSClose(fileRefNum);
		}
		return(theErr);
	}
	
	theErr = FSClose(fileRefNum);
	if (theErr != noErr)
	{
		CheckFileError(theErr, PSTR("Preferences"));
		return(theErr);
	}
	
	return(theErr);
}

//--------------------------------------------------------------  DeletePrefs

Boolean DeletePrefs (long *dirID, short *volRef)
{
	FSSpec		theSpecs;
	Str255		fileName;
	OSErr		theErr;

	PasStringCopy(kPrefFileName, fileName);
	
	theErr = FSMakeFSSpec(*volRef, *dirID, fileName, &theSpecs);
	if (theErr != noErr)
		return(false);
	else
		theErr = FSpDelete(&theSpecs);
	
	if (theErr != noErr)
		return(false);
	
	return(true);
}

//--------------------------------------------------------------  LoadPrefs

Boolean LoadPrefs (prefsInfo *thePrefs, short versionNeed)
{
	long		prefDirID;
	OSErr		theErr;
	short		systemVolRef;
	Boolean		noProblems;
	
	noProblems = GetPrefsFPath(&prefDirID, &systemVolRef);
	if (!noProblems)
		return(false);
	
	theErr = ReadPrefs(&prefDirID, &systemVolRef, thePrefs);
	if (theErr == eofErr)
	{
		BringUpDeletePrefsAlert();
		noProblems = DeletePrefs(&prefDirID, &systemVolRef);
		return (false);
	}
	else if (theErr != noErr)
		return (false);
	
	if (thePrefs->prefVersion != versionNeed)
	{
		BringUpDeletePrefsAlert();
		noProblems = DeletePrefs(&prefDirID, &systemVolRef);
		return(false);
	}
	
	return (true);
}

//--------------------------------------------------------------  BringUpDeletePrefsAlert

void BringUpDeletePrefsAlert (void)
{
	short		whoCares;
	
	InitCursor();
//	CenterAlert(kNewPrefsAlertID);
	whoCares = Alert(kNewPrefsAlertID, nil);
}

