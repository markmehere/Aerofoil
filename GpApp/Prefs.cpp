
//============================================================================
//----------------------------------------------------------------------------
//									Prefs.c
//----------------------------------------------------------------------------
//============================================================================

#include "PLPasStr.h"
#include "DialogManager.h"
#include "Externs.h"
#include "Environ.h"
#include "FileManager.h"
#include "IGpAudioDriver.h"
#include "IGpDisplayDriver.h"
#include "IGpInputDriver.h"
#include "IGpPrefsHandler.h"
#include "GpIOStream.h"
#include "MemoryManager.h"

#include "PLDrivers.h"


#define	kPrefCreatorType	'ozm5'
#define	kPrefFileType		'gliP'
#define	kPrefFileName		PSTR("Glider Prefs")
#define	kDefaultPrefFName	PSTR("Preferences")
#define kPrefsStringsID		160
#define kNewPrefsAlertID	160
#define	kPrefsFNameIndex	1

typedef struct modulePrefsListEntry
{
	THandle<char> m_identifier;
	THandle<uint8_t> m_contents;
	uint32_t m_version;
} modulePrefsListEntry;

Boolean CanUseFindFolder (void);
Boolean WritePrefs (const prefsInfo *, short versionNow, THandle<modulePrefsListEntry> *theModulePrefs);
PLError_t ReadPrefs (prefsInfo *thePrefs, short versionNeed, Boolean *isOldVersion, THandle<modulePrefsListEntry> *theModulePrefs);
Boolean DeletePrefs ();
void BringUpDeletePrefsAlert (void);


THandle<modulePrefsListEntry> theModulePrefs;


//==============================================================  Functions
//--------------------------------------------------------------  CanUseFindFolder

Boolean CanUseFindFolder (void)
{
	return true;
}

//--------------------------------------------------------------  WritePrefs

Boolean WritePrefs (const prefsInfo *thePrefs, short versionNow, THandle<modulePrefsListEntry> modulePrefs)
{
	PLError_t		theErr;
	GpIOStream *fileStream;
	long		byteCount;
	Str255		fileName;

	PortabilityLayer::FileManager *fm = PortabilityLayer::FileManager::GetInstance();

	PasStringCopy(kPrefFileName, fileName);

	VFileSpec theSpecs = MakeVFileSpec(PortabilityLayer::VirtualDirectories::kPrefs, fileName);

	theErr = fm->OpenNonCompositeFile(theSpecs.m_dir, theSpecs.m_name, ".dat", PortabilityLayer::EFilePermission_Write, GpFileCreationDispositions::kCreateOrOverwrite, fileStream);
	if (theErr != PLErrors::kNone)
	{
		CheckFileError(theErr, PSTR("Preferences"));
		return(false);
	}

	uint32_t version = versionNow;
	if (fileStream->Write(&version, sizeof(version)) != sizeof(version))
	{
		CheckFileError(PLErrors::kIOError, PSTR("Preferences"));
		fileStream->Close();
		return(false);
	}

	byteCount = sizeof(*thePrefs);

	if (fileStream->Write(thePrefs, byteCount) != byteCount)
	{
		CheckFileError(PLErrors::kIOError, PSTR("Preferences"));
		fileStream->Close();
		return(false);
	}

	uint32_t numEntries = 0;
	if (modulePrefs)
		numEntries = modulePrefs.MMBlock()->m_size / sizeof(modulePrefsListEntry);

	if (fileStream->Write(&numEntries, sizeof(numEntries)) != sizeof(numEntries))
	{
		CheckFileError(PLErrors::kIOError, PSTR("Preferences"));
		fileStream->Close();
		return(false);
	}

	for (uint32_t i = 0; i < numEntries; i++)
	{
		modulePrefsListEntry *entry = (*modulePrefs) + i;

		uint32_t version = entry->m_version;

		if (fileStream->Write(&version, sizeof(version)) != sizeof(version))
		{
			CheckFileError(PLErrors::kIOError, PSTR("Preferences"));
			fileStream->Close();
			return(false);
		}

		THandle<void> handles[2] = { entry->m_identifier.StaticCast<void>(), entry->m_contents.StaticCast<void>() };

		for (int hi = 0; hi < 2; hi++)
		{
			uint32_t hSize = handles[hi].MMBlock()->m_size;

			if (fileStream->Write(&hSize, sizeof(hSize)) != sizeof(hSize))
			{
				CheckFileError(PLErrors::kIOError, PSTR("Preferences"));
				fileStream->Close();
				return(false);
			}

			if (fileStream->Write(*handles[hi], hSize) != hSize)
			{
				CheckFileError(PLErrors::kIOError, PSTR("Preferences"));
				fileStream->Close();
				return(false);
			}
		}
	}

	fileStream->Close();

	return(true);
}

//--------------------------------------------------------------  SavePrefs

Boolean SavePrefs (prefsInfo *thePrefs, THandle<void> *modulePrefs, short versionNow)
{
	if (!WritePrefs(thePrefs, versionNow, modulePrefs->StaticCast<modulePrefsListEntry>()))
		return(false);

	return(true);
}

static void DestroyModulePrefs(THandle<modulePrefsListEntry> *theModulePrefs)
{
	if (!*theModulePrefs)
		return;

	modulePrefsListEntry *firstOldEntry = **theModulePrefs;

	if (!firstOldEntry)
		return;

	size_t numOldPrefs = theModulePrefs->MMBlock()->m_size / sizeof(modulePrefsListEntry);

	for (size_t i = 0; i < numOldPrefs; i++)
	{
		firstOldEntry[i].m_contents.Dispose();
		firstOldEntry[i].m_identifier.Dispose();
		firstOldEntry[i].~modulePrefsListEntry();
	}

	theModulePrefs->Dispose();
}

//--------------------------------------------------------------  ReadPrefs

PLError_t ReadPrefs (prefsInfo *thePrefs, short versionNeed, Boolean *isOldVersion, THandle<modulePrefsListEntry> *theModulePrefs)
{
	PLError_t	theErr;
	GpIOStream	*fileStream;
	long		byteCount;
	VFileSpec	theSpecs;
	Str255		fileName;

	*isOldVersion = false;

	PortabilityLayer::FileManager *fm = PortabilityLayer::FileManager::GetInstance();

	PasStringCopy(kPrefFileName, fileName);

	theSpecs = MakeVFileSpec(PortabilityLayer::VirtualDirectory_t::kPrefs, fileName);

	theErr = fm->OpenNonCompositeFile(theSpecs.m_dir, theSpecs.m_name, ".dat", PortabilityLayer::EFilePermission_Read, GpFileCreationDispositions::kOpenExisting, fileStream);
	if (theErr == PLErrors::kFileNotFound)
		return theErr;

	if (theErr != PLErrors::kNone)
	{
		CheckFileError(theErr, PSTR("Preferences"));
		return(theErr);
	}

	uint32_t version = 0;
	if (fileStream->Read(&version, sizeof(version)) != sizeof(version))
	{
		CheckFileError(theErr, PSTR("Preferences"));
		fileStream->Close();
		return(PLErrors::kIOError);
	}

	if (version != versionNeed)
	{
		*isOldVersion = true;
		fileStream->Close();
		return(PLErrors::kNone);
	}

	byteCount = sizeof(*thePrefs);

	if (fileStream->Read(thePrefs, byteCount) != byteCount)
	{
		CheckFileError(PLErrors::kIOError, PSTR("Preferences"));
		fileStream->Close();
		return(PLErrors::kIOError);
	}

	uint32_t numModulePrefs = 0;
	if (fileStream->Read(&numModulePrefs, sizeof(numModulePrefs)) != sizeof(numModulePrefs))
	{
		CheckFileError(PLErrors::kIOError, PSTR("Preferences"));
		fileStream->Close();
		return(PLErrors::kIOError);
	}

	DestroyModulePrefs(theModulePrefs);

	if (numModulePrefs)
	{
		PortabilityLayer::MemoryManager *mm = PortabilityLayer::MemoryManager::GetInstance();

		*theModulePrefs = THandle<modulePrefsListEntry>(mm->AllocHandle(numModulePrefs * sizeof(modulePrefsListEntry)));

		if (!*theModulePrefs)
		{
			CheckFileError(PLErrors::kIOError, PSTR("Preferences"));
			fileStream->Close();
			return(PLErrors::kIOError);
		}

		modulePrefsListEntry *firstNewEntry = **theModulePrefs;

		for (size_t i = 0; i < numModulePrefs; i++)
			new (firstNewEntry + i) modulePrefsListEntry();

		for (uint32_t i = 0; i < numModulePrefs; i++)
		{
			if (fileStream->Read(&firstNewEntry[i].m_version, sizeof(firstNewEntry[i].m_version)) != sizeof(firstNewEntry[i].m_version))
			{
				DestroyModulePrefs(theModulePrefs);
				CheckFileError(PLErrors::kIOError, PSTR("Preferences"));
				fileStream->Close();
				return(PLErrors::kIOError);
			}

			for (int subHandle = 0; subHandle < 2; subHandle++)
			{
				uint32_t handleSz;
				if (fileStream->Read(&handleSz, sizeof(handleSz)) != sizeof(handleSz))
				{
					DestroyModulePrefs(theModulePrefs);
					CheckFileError(PLErrors::kIOError, PSTR("Preferences"));
					fileStream->Close();
					return(PLErrors::kIOError);
				}

				PortabilityLayer::MMHandleBlock *mmBlock = mm->AllocHandle(handleSz);
				if (!mmBlock)
				{
					DestroyModulePrefs(theModulePrefs);
					CheckFileError(PLErrors::kIOError, PSTR("Preferences"));
					fileStream->Close();
					return(PLErrors::kIOError);
				}

				if (subHandle == 0)
					firstNewEntry[i].m_identifier = THandle<char>(mmBlock);
				else
					firstNewEntry[i].m_contents = THandle<uint8_t>(mmBlock);

				if (fileStream->Read(mmBlock->m_contents, handleSz) != handleSz)
				{
					DestroyModulePrefs(theModulePrefs);
					CheckFileError(PLErrors::kIOError, PSTR("Preferences"));
					fileStream->Close();
					return(PLErrors::kIOError);
				}
			}
		}
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

	return PortabilityLayer::FileManager::GetInstance()->DeleteNonCompositeFile(theSpecs.m_dir, theSpecs.m_name, ".dat");
}

//--------------------------------------------------------------  RunFunctionOnAllPrefsHandlers
bool RunFunctionOnAllPrefsHandlers (void *context, bool (*func) (void *context, IGpPrefsHandler *handler))
{
	IGpPrefsHandler *ddHandler = PLDrivers::GetDisplayDriver()->GetPrefsHandler();
	if (ddHandler && !func(context, ddHandler))
		return false;


	if (IGpAudioDriver *audioDriver = PLDrivers::GetAudioDriver())
	{
		IGpPrefsHandler *adHandler = audioDriver->GetPrefsHandler();
		if (adHandler && !func(context, adHandler))
			return false;
	}

	size_t numInputDrivers = PLDrivers::GetNumInputDrivers();

	for (size_t i = 0; i < numInputDrivers; i++)
	{
		IGpPrefsHandler *idHandler = PLDrivers::GetInputDriver(i)->GetPrefsHandler();
		if (idHandler && !func(context, idHandler))
			return false;
	}

	return true;
}



//--------------------------------------------------------------  LoadPrefs

Boolean LoadPrefs (prefsInfo *thePrefs, THandle<void> *modulePrefs, short versionNeed)
{
	PLError_t		theErr;
	Boolean		noProblems;
	Boolean		isOldVersion = 0;

	THandle<modulePrefsListEntry> mPrefs;

	theErr = ReadPrefs(thePrefs, versionNeed, &isOldVersion, &mPrefs);

	if (theErr == PLErrors::kFileNotFound)
		return (false);

	if (theErr != PLErrors::kNone)
	{
		BringUpDeletePrefsAlert();
		noProblems = DeletePrefs();
		return (false);
	}

	if (isOldVersion)
	{
		BringUpDeletePrefsAlert();
		noProblems = DeletePrefs();
		return(false);
	}

	*modulePrefs = mPrefs.StaticCast<void>();

	return (true);
}

//--------------------------------------------------------------  BringUpDeletePrefsAlert

static THandle<void> CloneHandle(THandle<void> hdl)
{
	if (!hdl)
		return THandle<void>();

	PortabilityLayer::MMHandleBlock *newHdl = PortabilityLayer::MemoryManager::GetInstance()->AllocHandle(hdl.MMBlock()->m_size);
	if (!newHdl)
		return THandle<void>();

	memcpy(newHdl->m_contents, hdl.MMBlock()->m_contents, hdl.MMBlock()->m_size);

	return THandle<void>(newHdl);
}

struct SaveModulePrefsContext
{
	size_t m_numModulePrefs;
	THandle<modulePrefsListEntry> m_newPrefs;
};

bool SaveModulePrefsWriteFunc(void *vcontext, const void *identifier, size_t identifierSize, const void *contents, size_t contentsSize, uint32_t version)
{
	PortabilityLayer::MemoryManager *mm = PortabilityLayer::MemoryManager::GetInstance();

	SaveModulePrefsContext *context = static_cast<SaveModulePrefsContext*>(vcontext);
	modulePrefsListEntry *saveToEntry = nullptr;

	for (size_t i = 0; i < context->m_numModulePrefs; i++)
	{
		modulePrefsListEntry *checkEntry = (*context->m_newPrefs) + i;
		THandle<char> candidateIdentHdl = checkEntry->m_identifier;
		size_t candidateIdentSize = candidateIdentHdl.MMBlock()->m_size;

		if (candidateIdentSize == identifierSize && !memcmp(*candidateIdentHdl, identifier, candidateIdentSize))
		{
			saveToEntry = checkEntry;
			break;
		}
	}

	if (!saveToEntry)
	{
		if (context->m_numModulePrefs == 0)
			context->m_newPrefs = THandle<modulePrefsListEntry>(mm->AllocHandle(sizeof(modulePrefsListEntry)));
		else
			mm->ResizeHandle(context->m_newPrefs.MMBlock(), (context->m_numModulePrefs + 1) * sizeof(modulePrefsListEntry));

		saveToEntry = (*context->m_newPrefs) + context->m_numModulePrefs;
		new (saveToEntry) modulePrefsListEntry();

		context->m_numModulePrefs++;
	}

	if (saveToEntry->m_contents)
		saveToEntry->m_contents.Dispose();

	if (!saveToEntry->m_identifier)
	{
		saveToEntry->m_identifier = THandle<char>(mm->AllocHandle(identifierSize));
		memcpy(*saveToEntry->m_identifier, identifier, identifierSize);
	}

	saveToEntry->m_contents = THandle<uint8_t>(mm->AllocHandle(contentsSize));
	memcpy(*saveToEntry->m_contents, contents, contentsSize);

	saveToEntry->m_version = version;

	return true;
}

bool SaveModulePrefsCallback(void *vcontext, IGpPrefsHandler *handler)
{
	return handler->SavePrefs(vcontext, SaveModulePrefsWriteFunc);
}

Boolean SaveModulePrefs(THandle<void> currentModulePrefs, THandle<void> *outModulePrefs)
{
	SaveModulePrefsContext context;

	size_t numModulePrefs = 0;
	THandle<modulePrefsListEntry> newPrefs;

	if (currentModulePrefs)
	{
		numModulePrefs = currentModulePrefs.MMBlock()->m_size / sizeof(modulePrefsListEntry);
		newPrefs = CloneHandle(currentModulePrefs).StaticCast<modulePrefsListEntry>();

		modulePrefsListEntry *entries = *newPrefs;
		for (size_t i = 0; i < numModulePrefs; i++)
		{
			entries[i].m_contents = CloneHandle(entries[i].m_contents.StaticCast<void>()).StaticCast<uint8_t>();
			entries[i].m_identifier = CloneHandle(entries[i].m_identifier.StaticCast<void>()).StaticCast<char>();
		}
	}
	else
		newPrefs = THandle<modulePrefsListEntry>(PortabilityLayer::MemoryManager::GetInstance()->AllocHandle(0));

	context.m_newPrefs = newPrefs;
	context.m_numModulePrefs = numModulePrefs;

	bool savedOK = RunFunctionOnAllPrefsHandlers(&context, SaveModulePrefsCallback);

	if (savedOK)
	{
		outModulePrefs->Dispose();
		*outModulePrefs = context.m_newPrefs.StaticCast<void>();
	}

	return savedOK;
}

//--------------------------------------------------------------  BringUpDeletePrefsAlert

struct ApplyModulePrefsContext
{
	size_t m_numModulePrefs;
	THandle<modulePrefsListEntry> m_newPrefs;
};

bool ApplyModulePrefsCallback(void *vcontext, IGpPrefsHandler *handler)
{
	ApplyModulePrefsContext *context = static_cast<ApplyModulePrefsContext*>(vcontext);

	const modulePrefsListEntry *entries = *(context->m_newPrefs);

	for (size_t i = 0; i < context->m_numModulePrefs; i++)
		handler->ApplyPrefs(*entries[i].m_identifier, entries[i].m_identifier.MMBlock()->m_size, *entries[i].m_contents, entries[i].m_contents.MMBlock()->m_size, entries[i].m_version);

	return true;
}

Boolean ApplyModulePrefs (THandle<void> *modulePrefs)
{
	ApplyModulePrefsContext context;

	context.m_newPrefs = modulePrefs->StaticCast<modulePrefsListEntry>();
	context.m_numModulePrefs = context.m_newPrefs.MMBlock()->m_size / sizeof(modulePrefsListEntry);

	return RunFunctionOnAllPrefsHandlers(&context, ApplyModulePrefsCallback);
}

//--------------------------------------------------------------  BringUpDeletePrefsAlert

void BringUpDeletePrefsAlert (void)
{
	short		whoCares;

	InitCursor();
//	CenterAlert(kNewPrefsAlertID);
	whoCares = PortabilityLayer::DialogManager::GetInstance()->DisplayAlert(kNewPrefsAlertID, nullptr);
}

