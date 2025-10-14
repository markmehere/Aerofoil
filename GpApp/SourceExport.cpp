#include "CombinedTimestamp.h"
#include "DeflateCodec.h"
#include "Environ.h"
#include "FontFamily.h"
#include "GpBuildVersion.h"
#include "GpIOStream.h"
#include "GpFileCreationDisposition.h"
#include "IGpDirectoryCursor.h"
#include "IGpDisplayDriver.h"
#include "IGpFileSystem.h"
#include "MemoryManager.h"
#include "RenderedFont.h"
#include "GpApplicationName.h"
#include "GpRenderedFontMetrics.h"
#include "ResolveCachingColor.h"
#include "Vec2i.h"
#include "WindowDef.h"
#include "WindowManager.h"
#include "ZipFile.h"

#include "PLCore.h"
#include "PLDrivers.h"
#include "PLStandardColors.h"
#include "PLSysCalls.h"

#include <vector>
#include <string>

struct SourceExportState
{
	SourceExportState();
	~SourceExportState();

	Window *m_window;
	Window *m_exclWindow;
	Rect m_progressRect;
	Rect m_filledProgress;
	GpIOStream *m_tsStream;
	GpIOStream *m_sourcePkgStream;
	GpIOStream *m_fStream;

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
	, m_exclWindow(nullptr)
	, m_progressRect(Rect::Create(0, 0, 0, 0))
	, m_tsStream(nullptr)
	, m_sourcePkgStream(nullptr)
	, m_fStream(nullptr)
	, m_dataProcessed(0)
	, m_dataTotal(0)
{
}

SourceExportState::~SourceExportState()
{
	if (m_window)
	{
		PortabilityLayer::WindowManager::GetInstance()->SwapExclusiveWindow(m_exclWindow);
		PortabilityLayer::WindowManager::GetInstance()->DestroyWindow(m_window);
	}

	CloseStreamIfOpen(m_tsStream);
	CloseStreamIfOpen(m_sourcePkgStream);
	CloseStreamIfOpen(m_fStream);
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

static void InitSourceExportWindow(SourceExportState *state)
{
	static const int kLoadScreenHeight = 32;
	static const int kLoadScreenWidth = 256;

	ForceSyncFrame();
	PLSysCalls::Sleep(1);

	// We have to use this instead of thisMac.fullScreen because the resolution may change during the sleep call, especially on Android displays where
	// the status bar dismissal causes a major change in the virtual resolution.
	PortabilityLayer::Vec2i displaySize = PortabilityLayer::WindowManager::GetInstance()->GetDisplayResolution();
	unsigned int displayWidth = displaySize.m_x;
	unsigned int displayHeight = displaySize.m_y;

	int32_t lsX = (static_cast<int32_t>(displayWidth) - kLoadScreenWidth) / 2;
	int32_t lsY = (static_cast<int32_t>(displayHeight) - kLoadScreenHeight) / 2;


	const Rect loadScreenRect = Rect::Create(lsY, lsX, lsY + kLoadScreenHeight, lsX + kLoadScreenWidth);
	const Rect loadScreenLocalRect = Rect::Create(0, 0, loadScreenRect.Height(), loadScreenRect.Width());

	PortabilityLayer::WindowDef def = PortabilityLayer::WindowDef::Create(loadScreenRect, PortabilityLayer::WindowStyleFlags::kAlert, true, 0, 0, PSTR(""));

	state->m_window = PortabilityLayer::WindowManager::GetInstance()->CreateWindow(def);
	state->m_exclWindow = state->m_window;

	PortabilityLayer::WindowManager::GetInstance()->SwapExclusiveWindow(state->m_exclWindow);

	PortabilityLayer::WindowManager::GetInstance()->PutWindowBehind(state->m_window, PL_GetPutInFrontWindowPtr());

	DrawSurface *surface = state->m_window->GetDrawSurface();
	PortabilityLayer::ResolveCachingColor blackColor(StdColors::Black());
	PortabilityLayer::ResolveCachingColor whiteColor(StdColors::White());

	surface->FillRect(loadScreenLocalRect, whiteColor);

	PortabilityLayer::WindowManager::GetInstance()->FlickerWindowIn(state->m_window, 32);

	const PLPasStr loadingText = PSTR("Exporting...");
	PortabilityLayer::RenderedFont *font = GetFont(PortabilityLayer::FontPresets::kApplication18);
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
	GpIOStream *stream = PLDrivers::GetFileSystem()->OpenFileNested(virtualDir, paths, numPaths, false, GpFileCreationDispositions::kOpenExisting);
	if (!stream)
		return false;

	const size_t sz = static_cast<size_t>(stream->Size());
	stream->Close();

	outSize = sz;
	return true;
}

static bool RetrieveCompositeDirSize(PortabilityLayer::VirtualDirectory_t virtualDir, size_t &totalSizeOut)
{
	size_t totalSize = 0;
	totalSizeOut = 0;

	IGpDirectoryCursor *dirCursor = PLDrivers::GetFileSystem()->ScanDirectory(virtualDir);
	if (!dirCursor)
		return false;

	const char *fname = nullptr;
	while (dirCursor->GetNext(fname))
	{
		size_t fnameLen = strlen(fname);
		if (fnameLen >= 4 && fname[fnameLen - 4] == '.' && fname[fnameLen - 3] == 'g' && fname[fnameLen - 2] == 'p' &&
				(fname[fnameLen - 1] == 'f' || fname[fnameLen - 1] == 'd' || fname[fnameLen - 1] == 'a'))
		{
			size_t fSize = 0;
			if (!RetrieveSingleFileSize(virtualDir, &fname, 1, fSize))
				return false;

			totalSize += fSize;
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

static bool RepackDirectory(SourceExportState &state, GpIOStream *outStream, std::vector<PortabilityLayer::ZipCentralDirectoryFileHeader> &centralDirFiles, std::vector<uint8_t> &fileNameChars, std::vector<size_t> &fileNameLengths, const char *storageDir, PortabilityLayer::VirtualDirectory_t virtualDir)
{
	PortabilityLayer::MemoryManager *mm = PortabilityLayer::MemoryManager::GetInstance();

	uint16_t dosDate = 0;
	uint16_t dosTime = 0;
	state.m_ts.GetAsMSDOSTimestamp(dosDate, dosTime);

	IGpDirectoryCursor *dirCursor = PLDrivers::GetFileSystem()->ScanDirectory(virtualDir);
	if (!dirCursor)
		return false;

	const size_t storageDirLength = strlen(storageDir);

	const char *fpath = nullptr;
	while (dirCursor->GetNext(fpath))
	{
		const size_t fpathLength = strlen(fpath);

		if (fpathLength < 4)
			continue;

		const char *extension = fpath + fpathLength - 4;
		if (extension[0] != '.' || extension[1] != 'g' || extension[2] != 'p')
			continue;

		if (extension[3] != 'a' && extension[3] != 'f' && extension[3] != 'd')
			continue;

		const bool shouldStore = (extension[3] == 'a');

		state.m_fStream = PLDrivers::GetFileSystem()->OpenFile(virtualDir, fpath, false, GpFileCreationDispositions::kOpenExisting);
		if (!state.m_fStream)
			return false;

		const size_t combinedPathLength = fpathLength + storageDirLength;

		PortabilityLayer::DeflateContext *defContext = nullptr;

		if (!shouldStore)
		{
			defContext = PortabilityLayer::DeflateContext::Create(outStream, 9);
			if (!defContext)
				return false;
		}

		const size_t fileSize = state.m_fStream->Size();

		GpUFilePos_t localHeaderPos = outStream->Tell();

		PortabilityLayer::ZipFileLocalHeader lHeader;
		memset(&lHeader, 0, sizeof(lHeader));

		if (outStream->Write(&lHeader, sizeof(lHeader)) != sizeof(lHeader))
			return false;

		if (outStream->Write(storageDir, storageDirLength) != storageDirLength || outStream->Write(fpath, fpathLength) != fpathLength)
			return false;

		GpUFilePos_t fileContentsPos = outStream->Tell();

		const size_t kCopyBufferSize = 1024;
		uint8_t copyBuffer[kCopyBufferSize];

		uint32_t crc = 0;
		for (size_t foffs = 0; foffs < fileSize; foffs += kCopyBufferSize)
		{
			size_t chunkSize = kCopyBufferSize;
			if (fileSize - foffs < chunkSize)
				chunkSize = fileSize - foffs;

			if (state.m_fStream->Read(copyBuffer, chunkSize) != chunkSize)
			{
				defContext->Destroy();
				return false;
			}

			crc = PortabilityLayer::DeflateContext::CRC32(crc, copyBuffer, chunkSize);

			if (defContext)
			{
				if (!defContext->Append(copyBuffer, chunkSize))
				{
					defContext->Destroy();
					return false;
				}
			}
			else
			{
				if (outStream->Write(copyBuffer, chunkSize) != chunkSize)
					return false;
			}
		}

		if (defContext && !defContext->Flush())
		{
			defContext->Destroy();
			return false;
		}

		if (defContext)
			defContext->Destroy();

		GpUFilePos_t compressedEndPos = outStream->Tell();
		GpUFilePos_t compressedSize = compressedEndPos - fileContentsPos;

		lHeader.m_signature = PortabilityLayer::ZipFileLocalHeader::kSignature;
		lHeader.m_versionRequired = shouldStore ? PortabilityLayer::ZipConstants::kStoredRequiredVersion : PortabilityLayer::ZipConstants::kCompressedRequiredVersion;
		lHeader.m_flags = 0;
		lHeader.m_method = shouldStore ? PortabilityLayer::ZipConstants::kStoredMethod : PortabilityLayer::ZipConstants::kDeflatedMethod;
		lHeader.m_modificationTime = dosTime;
		lHeader.m_modificationDate = dosDate;
		lHeader.m_crc = crc;
		lHeader.m_compressedSize = compressedSize;
		lHeader.m_uncompressedSize = fileSize;
		lHeader.m_fileNameLength = combinedPathLength;
		lHeader.m_extraFieldLength = 0;

		if (!outStream->SeekStart(localHeaderPos))
			return false;

		if (outStream->Write(&lHeader, sizeof(lHeader)) != sizeof(lHeader))
			return false;

		if (!outStream->SeekStart(compressedEndPos))
			return false;

		PortabilityLayer::ZipCentralDirectoryFileHeader cdirHeader;

		cdirHeader.m_signature = PortabilityLayer::ZipCentralDirectoryFileHeader::kSignature;
		cdirHeader.m_versionCreated = PortabilityLayer::ZipConstants::kCompressedRequiredVersion;
		cdirHeader.m_versionRequired = lHeader.m_versionRequired;
		cdirHeader.m_flags = 0;
		cdirHeader.m_method = lHeader.m_method;
		cdirHeader.m_modificationTime = lHeader.m_modificationTime;
		cdirHeader.m_modificationDate = lHeader.m_modificationDate;
		cdirHeader.m_crc = lHeader.m_crc;
		cdirHeader.m_compressedSize = lHeader.m_compressedSize;
		cdirHeader.m_uncompressedSize = lHeader.m_uncompressedSize;
		cdirHeader.m_fileNameLength = lHeader.m_fileNameLength;
		cdirHeader.m_extraFieldLength = lHeader.m_extraFieldLength;
		cdirHeader.m_commentLength = 0;
		cdirHeader.m_diskNumber = 0;
		cdirHeader.m_internalAttributes = 0;
		cdirHeader.m_externalAttributes = PortabilityLayer::ZipConstants::kArchivedAttributes;
		cdirHeader.m_localHeaderOffset = localHeaderPos;

		fileNameChars.resize(fileNameChars.size() + combinedPathLength);
		uint8_t *fileNameStorageLoc = &fileNameChars[fileNameChars.size() - combinedPathLength];

		memcpy(fileNameStorageLoc, storageDir, storageDirLength);
		memcpy(fileNameStorageLoc + storageDirLength, fpath, fpathLength);

		fileNameLengths.push_back(combinedPathLength);

		centralDirFiles.push_back(cdirHeader);

		state.m_dataProcessed += fileSize;

		UpdateProgress(state);
	}

	dirCursor->Destroy();

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

static bool AddZipDirectory(GpIOStream *stream, std::vector<PortabilityLayer::ZipCentralDirectoryFileHeader> &centralDirFiles, std::vector<uint8_t> &fileNameChars, std::vector<size_t> &fileNameLengths, const char *path, const PortabilityLayer::CombinedTimestamp &ts)
{
	size_t nameLength = strlen(path);

	uint16_t dosDate = 0;
	uint16_t dosTime = 0;
	ts.GetAsMSDOSTimestamp(dosDate, dosTime);

	GpUFilePos_t localHeaderPos = stream->Tell();

	PortabilityLayer::ZipFileLocalHeader localHeader;
	localHeader.m_signature = PortabilityLayer::ZipFileLocalHeader::kSignature;
	localHeader.m_versionRequired = PortabilityLayer::ZipConstants::kDirectoryRequiredVersion;
	localHeader.m_flags = 0;
	localHeader.m_method = PortabilityLayer::ZipConstants::kStoredMethod;
	localHeader.m_modificationTime = dosTime;
	localHeader.m_modificationDate = dosDate;
	localHeader.m_crc = 0;
	localHeader.m_compressedSize = 0;
	localHeader.m_uncompressedSize = 0;
	localHeader.m_fileNameLength = nameLength;
	localHeader.m_extraFieldLength = 0;

	if (stream->Write(&localHeader, sizeof(localHeader)) != sizeof(localHeader))
		return false;

	if (stream->Write(path, nameLength) != nameLength)
		return false;

	PortabilityLayer::ZipCentralDirectoryFileHeader cdirHeader;
	cdirHeader.m_signature = PortabilityLayer::ZipCentralDirectoryFileHeader::kSignature;
	cdirHeader.m_versionCreated = PortabilityLayer::ZipConstants::kCompressedRequiredVersion;
	cdirHeader.m_versionRequired = PortabilityLayer::ZipConstants::kDirectoryRequiredVersion;
	cdirHeader.m_flags = 0;
	cdirHeader.m_method = PortabilityLayer::ZipConstants::kStoredMethod;
	cdirHeader.m_modificationTime = dosTime;
	cdirHeader.m_modificationDate = dosDate;
	cdirHeader.m_crc = 0;
	cdirHeader.m_compressedSize = 0;
	cdirHeader.m_uncompressedSize = 0;
	cdirHeader.m_fileNameLength = nameLength;
	cdirHeader.m_extraFieldLength = 0;
	cdirHeader.m_commentLength = 0;
	cdirHeader.m_diskNumber = 0;
	cdirHeader.m_internalAttributes = 0;
	cdirHeader.m_externalAttributes = PortabilityLayer::ZipConstants::kDirectoryAttributes;
	cdirHeader.m_localHeaderOffset = localHeaderPos;

	centralDirFiles.push_back(cdirHeader);

	for (size_t i = 0; i < nameLength; i++)
		fileNameChars.push_back(reinterpret_cast<const uint8_t*>(path)[i]);

	fileNameLengths.push_back(nameLength);

	return true;
}

void DoExportSourceCode (void)
{
	PLDrivers::GetFileSystem()->OpenGithub();
}
