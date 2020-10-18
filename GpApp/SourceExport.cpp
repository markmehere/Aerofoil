#include "CombinedTimestamp.h"
#include "DeflateCodec.h"
#include "Environ.h"
#include "GpIOStream.h"
#include "HostDirectoryCursor.h"
#include "HostFileSystem.h"
#include "MemoryManager.h"
#include "PLCore.h"
#include "PLStandardColors.h"
#include "PLSysCalls.h"
#include "RenderedFont.h"
#include "GpRenderedFontMetrics.h"
#include "ResolveCachingColor.h"
#include "ZipFile.h"
#include "WindowDef.h"
#include "WindowManager.h"
#include "FontFamily.h"

#include <vector>
#include <string>

struct SourceExportState
{
	SourceExportState();
	~SourceExportState();

	Window *m_window;
	Rect m_progressRect;
	Rect m_filledProgress;
	GpIOStream *m_tsStream;
	GpIOStream *m_sourcePkgStream;
	GpIOStream *m_looseStream;

	size_t m_dataProcessed;
	size_t m_dataTotal;

	PortabilityLayer::CombinedTimestamp m_ts;

private:
	static void CloseStreamIfOpen(GpIOStream *stream)
	{
		if (stream)
			stream->Close();
	}
};

SourceExportState::SourceExportState()
	: m_window(nullptr)
	, m_progressRect(Rect::Create(0, 0, 0, 0))
	, m_tsStream(nullptr)
	, m_sourcePkgStream(nullptr)
	, m_looseStream(nullptr)
	, m_dataProcessed(0)
	, m_dataTotal(0)
{
}

SourceExportState::~SourceExportState()
{
	if (m_window)
		PortabilityLayer::WindowManager::GetInstance()->DestroyWindow(m_window);

	CloseStreamIfOpen(m_tsStream);
	CloseStreamIfOpen(m_sourcePkgStream);
	CloseStreamIfOpen(m_looseStream);
}

struct SortableEntry
{
	std::string m_zipLocation;
	PortabilityLayer::VirtualDirectory_t m_virtualDirectory;
	PortabilityLayer::PascalStr<255> m_filename;

	static SortableEntry Create(const char *zipLocation, PortabilityLayer::VirtualDirectory_t virtualDir, const PLPasStr &filename);
};

SortableEntry SortableEntry::Create(const char *zipLocation, PortabilityLayer::VirtualDirectory_t virtualDir, const PLPasStr &filename)
{
	SortableEntry entry;
	entry.m_zipLocation = zipLocation;
	entry.m_virtualDirectory = virtualDir;
	entry.m_filename.Set(filename.Length(), filename.Chars());

	return entry;
}

static void ConvertToMSDOSTimestamp(const PortabilityLayer::CombinedTimestamp &ts, uint16_t &msdosDate, uint16_t &msdosTime)
{
	int32_t yearsSince1980 = ts.GetLocalYear() - 1980;
	uint8_t month = ts.m_localMonth;
	uint8_t day = ts.m_localDay;

	uint8_t hour = ts.m_localHour;
	uint8_t minute = ts.m_localMinute;
	uint8_t second = ts.m_localSecond;

	if (yearsSince1980 < 0)
	{
		// Time machine
		yearsSince1980 = 0;
		second = 0;
		minute = 0;
		hour = 0;
		day = 1;
		month = 1;
	}
	else if (yearsSince1980 > 127)
	{
		// I was promised flying cars, but it's 2107 and you're still flying paper airplanes...
		yearsSince1980 = 127;
		second = 59;
		minute = 59;
		hour = 23;
		day = 31;
		month = 12;
	}

	msdosTime = (second / 2) | (minute << 5) | (hour << 11);
	msdosDate = day | (month << 5) | (yearsSince1980 << 9);
}

static void InitSourceExportWindow(SourceExportState *state)
{
	static const int kLoadScreenHeight = 32;
	static const int kLoadScreenWidth = 256;

	ForceSyncFrame();
	PLSysCalls::Sleep(1);

	int32_t lsX = (thisMac.fullScreen.Width() - kLoadScreenWidth) / 2;
	int32_t lsY = (thisMac.fullScreen.Height() - kLoadScreenHeight) / 2;


	const Rect loadScreenRect = Rect::Create(lsY, lsX, lsY + kLoadScreenHeight, lsX + kLoadScreenWidth);
	const Rect loadScreenLocalRect = Rect::Create(0, 0, loadScreenRect.Height(), loadScreenRect.Width());

	PortabilityLayer::WindowDef def = PortabilityLayer::WindowDef::Create(loadScreenRect, PortabilityLayer::WindowStyleFlags::kAlert, true, 0, 0, PSTR(""));

	state->m_window = PortabilityLayer::WindowManager::GetInstance()->CreateWindow(def);
	PortabilityLayer::WindowManager::GetInstance()->PutWindowBehind(state->m_window, PL_GetPutInFrontWindowPtr());

	DrawSurface *surface = state->m_window->GetDrawSurface();
	PortabilityLayer::ResolveCachingColor blackColor(StdColors::Black());
	PortabilityLayer::ResolveCachingColor whiteColor(StdColors::White());

	surface->FillRect(loadScreenLocalRect, whiteColor);

	PortabilityLayer::WindowManager::GetInstance()->FlickerWindowIn(state->m_window, 32);

	const PLPasStr loadingText = PSTR("Exporting...");
	PortabilityLayer::RenderedFont *font = GetApplicationFont(18, PortabilityLayer::FontFamilyFlag_None, true);
	int32_t textY = (kLoadScreenHeight + font->GetMetrics().m_ascent) / 2;
	surface->DrawString(Point::Create(4 + 16, textY), loadingText, blackColor, font);

	static const int32_t loadBarPadding = 16;
	int32_t loadBarStartX = static_cast<int32_t>(font->MeasureString(loadingText.UChars(), loadingText.Length())) + 4 + 16 + loadBarPadding;
	int32_t loadBarEndX = loadScreenLocalRect.right - loadBarPadding;

	state->m_progressRect = Rect::Create((loadScreenLocalRect.Height() - 8) / 2, loadBarStartX, (loadScreenLocalRect.Height() + 8) / 2, loadBarEndX);
	state->m_filledProgress = Rect::Create(state->m_progressRect.top, state->m_progressRect.left, state->m_progressRect.bottom, state->m_progressRect.left);

	surface->FrameRect(state->m_progressRect, blackColor);
}

static bool RetrieveSingleFileSize(PortabilityLayer::VirtualDirectory_t virtualDir, char const* const* paths, size_t numPaths, size_t &outSize)
{
	GpIOStream *stream = PortabilityLayer::HostFileSystem::GetInstance()->OpenFileNested(virtualDir, paths, numPaths, false, GpFileCreationDispositions::kOpenExisting);
	if (!stream)
		return false;

	const size_t sz = static_cast<size_t>(stream->Size());
	stream->Close();

	outSize = sz;
	return true;
}

static bool RetrieveResourceTypeDirSize(PortabilityLayer::VirtualDirectory_t virtualDir, const char *arcDir, const char *typeDir, size_t &totalSizeOut)
{
	size_t totalSize = 0;

	const char *nestedDirs[2] = { arcDir, typeDir };

	PortabilityLayer::HostDirectoryCursor *dirCursor = PortabilityLayer::HostFileSystem::GetInstance()->ScanDirectoryNested(virtualDir, nestedDirs, 2);
	if (!dirCursor)
		return false;

	const char *fname = nullptr;
	while (dirCursor->GetNext(fname))
	{
		const char *nestedFile[3] = { arcDir, typeDir, fname };

		size_t fileSize = 0;
		if (!RetrieveSingleFileSize(virtualDir, nestedFile, 3, fileSize))
			return false;

		totalSize += fileSize;
	}

	dirCursor->Destroy();

	totalSizeOut = totalSize;

	return true;
}

static bool RetrieveResourceDirSize(PortabilityLayer::VirtualDirectory_t virtualDir, const char *arcDir, size_t arcDirLen, size_t &totalSizeOut)
{
	size_t totalSize = 0;

	char *subDir = static_cast<char*>(PortabilityLayer::MemoryManager::GetInstance()->Alloc(arcDirLen + 1));
	memcpy(subDir, arcDir, arcDirLen);
	subDir[arcDirLen] = '\0';

	PortabilityLayer::HostDirectoryCursor *dirCursor = PortabilityLayer::HostFileSystem::GetInstance()->ScanDirectoryNested(virtualDir, &subDir, 1);
	if (!dirCursor)
	{
		// It's okay for the resource dir to not exist
		totalSizeOut = 0;
		return true;
	}

	const char *fname = nullptr;
	while (dirCursor->GetNext(fname))
	{
		size_t resTypeDirSize = 0;
		if (!RetrieveResourceTypeDirSize(virtualDir, subDir, fname, resTypeDirSize))
			return false;

		totalSize += resTypeDirSize;
	}

	dirCursor->Destroy();

	PortabilityLayer::MemoryManager::GetInstance()->Release(subDir);

	totalSizeOut = totalSize;

	return true;
}

static bool RetrieveCompositeDirSize(PortabilityLayer::VirtualDirectory_t virtualDir, size_t &totalSizeOut)
{
	size_t totalSize = 0;
	totalSizeOut = 0;

	PortabilityLayer::HostDirectoryCursor *dirCursor = PortabilityLayer::HostFileSystem::GetInstance()->ScanDirectory(virtualDir);
	if (!dirCursor)
		return false;

	const char *fname = nullptr;
	while (dirCursor->GetNext(fname))
	{
		size_t fnameLen = strlen(fname);
		if (fnameLen >= 4 && !memcmp(fname + fnameLen - 4, ".gpf", 4))
		{
			size_t resDirSize = 0;
			if (!RetrieveResourceDirSize(virtualDir, fname, fnameLen - 4, resDirSize))
				return false;

			totalSize += resDirSize;

			size_t gpfSize = 0;
			if (!RetrieveSingleFileSize(virtualDir, &fname, 1, gpfSize))
				return false;

			totalSize += gpfSize;
		}
		else if (fnameLen >= 4 && !memcmp(fname + fnameLen - 4, ".gpd", 4))
		{
			size_t gpdSize = 0;
			if (!RetrieveSingleFileSize(virtualDir, &fname, 1, gpdSize))
				return false;

			totalSize += gpdSize;
		}
	}

	totalSizeOut = totalSize;

	return true;
}

static void UpdateProgress(SourceExportState &state)
{
	uint32_t progressWidth = state.m_progressRect.Width();
	uint32_t oldFillWidth = state.m_filledProgress.Width();
	uint32_t newFillWidth = static_cast<uint64_t>(state.m_dataProcessed) * static_cast<uint64_t>(progressWidth) / state.m_dataTotal;
	if (newFillWidth > progressWidth)
		newFillWidth = progressWidth;

	if (newFillWidth > oldFillWidth)
	{
		state.m_filledProgress.right = state.m_filledProgress.left + static_cast<int32_t>(newFillWidth);
		Rect fillInRect = Rect::Create(state.m_progressRect.top, state.m_progressRect.left + oldFillWidth, state.m_progressRect.bottom, state.m_progressRect.left + newFillWidth);

		PortabilityLayer::ResolveCachingColor blackColor(StdColors::Black());
		state.m_window->GetDrawSurface()->FillRect(fillInRect, blackColor);

		PLSysCalls::Sleep(1);
	}
}

static bool RepackSourcePackage(SourceExportState &state, GpIOStream *outStream, std::vector<PortabilityLayer::ZipCentralDirectoryFileHeader> &centralDirFiles, std::vector<uint8_t> &fileNameChars, std::vector<size_t> &fileNameLengths)
{
	PortabilityLayer::MemoryManager *mm = PortabilityLayer::MemoryManager::GetInstance();

	for (;;)
	{
		LEUInt32_t signature;

		GpUFilePos_t inSigPos = state.m_sourcePkgStream->Tell();

		if (state.m_sourcePkgStream->Read(&signature, sizeof(signature)) != sizeof(signature))
			return false;

		if (signature != PortabilityLayer::ZipFileLocalHeader::kSignature)
		{
			// Trim central dir size off of the data estimate
			state.m_dataTotal -= (state.m_sourcePkgStream->Size() - inSigPos);
			return true;
		}

		PortabilityLayer::ZipFileLocalHeader lHeader;
		lHeader.m_signature = signature;

		if (state.m_sourcePkgStream->Read(reinterpret_cast<uint8_t*>(&lHeader) + sizeof(signature), sizeof(lHeader) - sizeof(signature)) != sizeof(lHeader) - sizeof(signature))
			return false;

		state.m_dataProcessed += sizeof(lHeader);

		GpUFilePos_t outLocalHeaderPos = outStream->Tell();
		if (outStream->Write(&lHeader, sizeof(lHeader)) != sizeof(lHeader))
			return false;

		const size_t fileNameLength = lHeader.m_fileNameLength;
		const size_t nameAndExtraFieldLength = fileNameLength + lHeader.m_extraFieldLength;
		uint8_t *nameAndExtraField = static_cast<uint8_t*>(mm->Alloc(nameAndExtraFieldLength));
		if (!nameAndExtraField)
			return false;

		if (state.m_sourcePkgStream->Read(nameAndExtraField, nameAndExtraFieldLength) != nameAndExtraFieldLength)
		{
			mm->Release(nameAndExtraField);
			return false;
		}

		state.m_dataProcessed += nameAndExtraFieldLength;

		fileNameChars.reserve(fileNameLength);
		for (size_t i = 0; i < fileNameLength; i++)
			fileNameChars.push_back(nameAndExtraField[i]);

		fileNameLengths.push_back(fileNameLength);

		if (!outStream->Write(nameAndExtraField, nameAndExtraFieldLength))
			return false;

		const bool endsInSlash = (nameAndExtraField[fileNameLength - 1] == '/');

		mm->Release(nameAndExtraField);

		assert(lHeader.m_method == PortabilityLayer::ZipConstants::kStoredMethod);

		size_t uncompressedSize = lHeader.m_uncompressedSize;
		size_t compressedSize = 0;

		GpUFilePos_t compressedDataStart = outStream->Tell();

		uint8_t copyBuffer[2048];

		bool isCompressed = false;
		if (uncompressedSize != 0)
		{
			isCompressed = true;

			PortabilityLayer::DeflateContext *context = PortabilityLayer::DeflateContext::Create(outStream, 9);
			if (!context)
				return false;

			size_t compressRemaining = uncompressedSize;
			while (compressRemaining > 0)
			{
				size_t chunkSize = compressRemaining;
				if (chunkSize > sizeof(copyBuffer))
					chunkSize = sizeof(copyBuffer);

				if (state.m_sourcePkgStream->Read(copyBuffer, chunkSize) != chunkSize || !context->Append(copyBuffer, chunkSize))
				{
					context->Destroy();
					return false;
				}

				compressRemaining -= chunkSize;
			}

			if (!context->Flush())
			{
				context->Destroy();
				return false;
			}

			context->Destroy();
		}

		GpUFilePos_t compressedDataEnd = outStream->Tell();

		lHeader.m_compressedSize = static_cast<uint32_t>(compressedDataEnd - compressedDataStart);
		if (isCompressed)
		{
			lHeader.m_method = PortabilityLayer::ZipConstants::kDeflatedMethod;
			lHeader.m_versionRequired = PortabilityLayer::ZipConstants::kCompressedRequiredVersion;

			if (!outStream->SeekStart(outLocalHeaderPos))
				return false;

			if (outStream->Write(&lHeader, sizeof(lHeader)) != sizeof(lHeader))
				return false;

			if (!outStream->SeekStart(compressedDataEnd))
				return false;
		}

		state.m_dataProcessed += uncompressedSize;

		const bool isDirectory = (lHeader.m_uncompressedSize == 0 && endsInSlash);

		PortabilityLayer::ZipCentralDirectoryFileHeader cdirHeader;
		cdirHeader.m_signature = PortabilityLayer::ZipCentralDirectoryFileHeader::kSignature;
		cdirHeader.m_versionCreated = PortabilityLayer::ZipConstants::kCompressedRequiredVersion;
		cdirHeader.m_versionRequired = isDirectory ? PortabilityLayer::ZipConstants::kDirectoryRequiredVersion : lHeader.m_versionRequired;
		cdirHeader.m_flags = 0;
		cdirHeader.m_method = lHeader.m_method;
		cdirHeader.m_modificationTime = lHeader.m_modificationTime;
		cdirHeader.m_modificationDate = lHeader.m_modificationDate;
		cdirHeader.m_crc = lHeader.m_crc;
		cdirHeader.m_compressedSize = lHeader.m_compressedSize;
		cdirHeader.m_uncompressedSize = lHeader.m_uncompressedSize;
		cdirHeader.m_fileNameLength = lHeader.m_fileNameLength;
		cdirHeader.m_extraFieldLength = 0;
		cdirHeader.m_commentLength = 0;
		cdirHeader.m_diskNumber = 0;
		cdirHeader.m_internalAttributes = 0;
		cdirHeader.m_externalAttributes = isDirectory ? PortabilityLayer::ZipConstants::kDirectoryAttributes : PortabilityLayer::ZipConstants::kArchivedAttributes;
		cdirHeader.m_localHeaderOffset = outLocalHeaderPos;

		centralDirFiles.push_back(cdirHeader);

		UpdateProgress(state);
	}

	return true;
}

static bool WriteCentralDirectory(GpIOStream *stream, const std::vector<PortabilityLayer::ZipCentralDirectoryFileHeader> &centralDirFiles, const std::vector<uint8_t> &fileNameChars, const std::vector<size_t> &fileNameLengths)
{
	const size_t numEntries = centralDirFiles.size();
	assert(fileNameLengths.size() == numEntries);

	GpUFilePos_t cdirLoc = stream->Tell();

	size_t fnameOffset = 0;
	for (size_t i = 0; i < numEntries; i++)
	{
		const PortabilityLayer::ZipCentralDirectoryFileHeader &cdirHeader = centralDirFiles[i];

		if (stream->Write(&cdirHeader, sizeof(cdirHeader)) != sizeof(cdirHeader))
			return false;

		const size_t fnameLength = fileNameLengths[i];
		if (stream->Write(&fileNameChars[fnameOffset], fnameLength) != fnameLength)
			return false;

		fnameOffset += fnameLength;
	}

	GpUFilePos_t cdirEnd = stream->Tell();

	PortabilityLayer::ZipEndOfCentralDirectoryRecord ecdRec;
	ecdRec.m_signature = PortabilityLayer::ZipEndOfCentralDirectoryRecord::kSignature;
	ecdRec.m_thisDiskNumber = 0;
	ecdRec.m_centralDirDisk = 0;
	ecdRec.m_numCentralDirRecordsThisDisk = centralDirFiles.size();
	ecdRec.m_numCentralDirRecords = centralDirFiles.size();
	ecdRec.m_centralDirectorySizeBytes = cdirEnd - cdirLoc;
	ecdRec.m_centralDirStartOffset = cdirLoc;
	ecdRec.m_commentLength = 0;

	if (stream->Write(&ecdRec, sizeof(ecdRec)) != sizeof(ecdRec))
		return false;

	return true;
}

bool ExportSourceToStream (GpIOStream *stream)
{
	SourceExportState state;
	InitSourceExportWindow(&state);

	state.m_tsStream = PortabilityLayer::HostFileSystem::GetInstance()->OpenFile(PortabilityLayer::VirtualDirectories::kApplicationData, "DefaultTimestamp.timestamp", false, GpFileCreationDispositions::kOpenExisting);
	if (!state.m_tsStream)
		return false;

	// Read timestamp
	const bool readTSOK = (state.m_tsStream->Read(&state.m_ts, sizeof(state.m_ts)) != sizeof(state.m_ts));
	state.m_tsStream->Close();
	state.m_tsStream = nullptr;

	state.m_sourcePkgStream = PortabilityLayer::HostFileSystem::GetInstance()->OpenFile(PortabilityLayer::VirtualDirectories::kApplicationData, "SourceCode.pkg", false, GpFileCreationDispositions::kOpenExisting);
	if (!state.m_sourcePkgStream)
		return false;

	PLSysCalls::ForceSyncFrame();
	PLSysCalls::Sleep(1);

	size_t sourcePkgSize = state.m_sourcePkgStream->Size();
	size_t looseFilesSize = 0;
	if (!RetrieveCompositeDirSize(PortabilityLayer::VirtualDirectories::kGameData, looseFilesSize))
		return false;

	PLSysCalls::ForceSyncFrame();
	PLSysCalls::Sleep(1);

	size_t applicationDataSize = 0;
	const char *applicationDataPath = "ApplicationResources";
	if (!RetrieveResourceDirSize(PortabilityLayer::VirtualDirectories::kApplicationData, applicationDataPath, strlen(applicationDataPath), applicationDataSize))
		return false;

	PLSysCalls::ForceSyncFrame();
	PLSysCalls::Sleep(1);

	state.m_dataTotal = applicationDataSize + looseFilesSize + sourcePkgSize;

	std::vector<PortabilityLayer::ZipCentralDirectoryFileHeader> centralDirFiles;
	std::vector<uint8_t> fileNameChars;
	std::vector<size_t> fileNameLengths;

	RepackSourcePackage(state, stream, centralDirFiles, fileNameChars, fileNameLengths);

	if (!WriteCentralDirectory(stream, centralDirFiles, fileNameChars, fileNameLengths))
		return false;

	PortabilityLayer::WindowManager::GetInstance()->FlickerWindowOut(state.m_window, 32);

	return true;
}

void DoExportSourceCode (void)
{
	GpIOStream *stream = PortabilityLayer::HostFileSystem::GetInstance()->OpenFile(PortabilityLayer::VirtualDirectories::kPrefs, "SourceExport.zip", true, GpFileCreationDispositions::kCreateOrOverwrite);
	if (!stream)
		return;

	ExportSourceToStream(stream);
	stream->Close();
}
