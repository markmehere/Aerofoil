
//============================================================================
//----------------------------------------------------------------------------
//									Prefs.c
//----------------------------------------------------------------------------
//============================================================================

#include "PLPasStr.h"
#include "Externs.h"
#include "Environ.h"
#include "FileManager.h"
#include "IOStream.h"


#define	kPrefCreatorType	'ozm5'
#define	kPrefFileType		'gliP'
#define	kPrefFileName		PSTR("Glider Prefs")
#define	kDefaultPrefFName	PSTR("Preferences")
#define kPrefsStringsID		160
#define kNewPrefsAlertID	160
#define	kPrefsFNameIndex	1


Boolean CanUseFindFolder (void);
Boolean WritePrefs (const prefsInfo *);
PLError_t ReadPrefs (prefsInfo *);
Boolean DeletePrefs ();
void BringUpDeletePrefsAlert (void);


//==============================================================  Functions
//--------------------------------------------------------------  CanUseFindFolder

Boolean CanUseFindFolder (void)
{
	return true;
}

//--------------------------------------------------------------  WritePrefs

Boolean WritePrefs (const prefsInfo *thePrefs)
{
	PLError_t		theErr;
	PortabilityLayer::IOStream *fileStream;
	long		byteCount;
	Str255		fileName;
	
	PasStringCopy(kPrefFileName, fileName);
	
	VFileSpec theSpecs = MakeVFileSpec(PortabilityLayer::VirtualDirectories::kPrefs, fileName);
	if (!PortabilityLayer::FileManager::GetInstance()->FileExists(PortabilityLayer::VirtualDirectories::kPrefs, fileName))
	{
		theErr = FSpCreate(theSpecs, kPrefCreatorType, kPrefFileType);
		if (theErr != PLErrors::kNone)
		{
			CheckFileError(theErr, PSTR("Preferences"));
			return(false);
		}
	}
	theErr = FSpOpenDF(theSpecs, fsRdWrPerm, fileStream);
	if (theErr != PLErrors::kNone)
	{
		CheckFileError(theErr, PSTR("Preferences"));
		return(false);
	}
	
	byteCount = sizeof(*thePrefs);

	if (fileStream->Write(thePrefs, byteCount) != byteCount)
	{
		CheckFileError(PLErrors::kIOError, PSTR("Preferences"));
		return(false);
	}

	fileStream->Close();
	
	return(true);
}

//--------------------------------------------------------------  SavePrefs

Boolean SavePrefs (prefsInfo *thePrefs, short versionNow)
{
	thePrefs->prefVersion = versionNow;
	
	if (!WritePrefs(thePrefs))
		return(false);
	
	return(true);
}

//--------------------------------------------------------------  ReadPrefs

PLError_t ReadPrefs (prefsInfo *thePrefs)
{
	PLError_t		theErr;
	PortabilityLayer::IOStream		*fileStream;
	long		byteCount;
	VFileSpec	theSpecs;
	Str255		fileName;
	
	PasStringCopy(kPrefFileName, fileName);
	
	theSpecs = MakeVFileSpec(PortabilityLayer::VirtualDirectory_t::kPrefs, fileName);

	if (!PortabilityLayer::FileManager::GetInstance()->FileExists(theSpecs.m_dir, theSpecs.m_name))
		return PLErrors::kFileNotFound;
	
	theErr = FSpOpenDF(theSpecs, fsRdWrPerm, fileStream);
	if (theErr != PLErrors::kNone)
	{
		CheckFileError(theErr, PSTR("Preferences"));
		return(theErr);
	}
	
	byteCount = sizeof(*thePrefs);

	if (fileStream->Read(thePrefs, byteCount) != byteCount)
	{
		CheckFileError(PLErrors::kIOError, PSTR("Preferences"));
		fileStream->Close();
		return(theErr);
	}
	
	fileStream->Close();

	return(theErr);
}

//--------------------------------------------------------------  DeletePrefs

Boolean DeletePrefs ()
{
	VFileSpec	theSpecs;
	Str255		fileName;
	PLError_t		theErr;

	PasStringCopy(kPrefFileName, fileName);
	
	theSpecs = MakeVFileSpec(PortabilityLayer::VirtualDirectories::kPrefs, fileName);

	theErr = FSpDelete(theSpecs);
	
	if (theErr != PLErrors::kNone)
		return(false);
	
	return(true);
}

//--------------------------------------------------------------  LoadPrefs

Boolean LoadPrefs (prefsInfo *thePrefs, short versionNeed)
{
	PLError_t		theErr;
	Boolean		noProblems;
	
	theErr = ReadPrefs(thePrefs);

	if (theErr == PLErrors::kFileNotFound)
		return (false);

	if (theErr != PLErrors::kNone)
	{
		BringUpDeletePrefsAlert();
		noProblems = DeletePrefs();
		return (false);
	}
	
	if (thePrefs->prefVersion != versionNeed)
	{
		BringUpDeletePrefsAlert();
		noProblems = DeletePrefs();
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

