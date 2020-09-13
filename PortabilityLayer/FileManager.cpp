#include "FileManager.h"

#include "FileBrowserUI.h"
#include "HostFileSystem.h"
#include "HostMemoryBuffer.h"
#include "MemReaderStream.h"
#include "MacBinary2.h"
#include "MacFileMem.h"
#include "PLPasStr.h"
#include "PLErrorCodes.h"
#include "PLSysCalls.h"
#include "ResTypeID.h"
#include "HostSystemServices.h"

#include <vector>

namespace PortabilityLayer
{
	class VirtualFile;

	class FileManagerImpl final : public FileManager
	{
	public:
		bool FileExists(VirtualDirectory_t dirID, const PLPasStr &filename) override;
		bool FileLocked(VirtualDirectory_t dirID, const PLPasStr &filename) override;
		bool DeleteFile(VirtualDirectory_t dirID, const PLPasStr &filename) override;

		PLError_t CreateFile(VirtualDirectory_t dirID, const PLPasStr &filename, const MacFileProperties &mfp) override;
		PLError_t CreateFileAtCurrentTime(VirtualDirectory_t dirID, const PLPasStr &filename, const ResTypeID &fileCreator, const ResTypeID &fileType) override;

		PLError_t OpenFileData(VirtualDirectory_t dirID, const PLPasStr &filename, EFilePermission filePermission, GpIOStream *&outRefNum) override;
		PLError_t OpenFileResources(VirtualDirectory_t dirID, const PLPasStr &filename, EFilePermission filePermission, GpIOStream *&outRefNum) override;
		bool ReadFileProperties(VirtualDirectory_t dirID, const PLPasStr &filename, MacFileProperties &properties) override;

		PLError_t RawOpenFileData(VirtualDirectory_t dirID, const PLPasStr &filename, EFilePermission filePermission, bool ignoreMeta, GpFileCreationDisposition_t creationDisposition, GpIOStream *&outStream) override;
		PLError_t RawOpenFileResources(VirtualDirectory_t dirID, const PLPasStr &filename, EFilePermission filePermission, bool ignoreMeta, GpFileCreationDisposition_t creationDisposition, GpIOStream *&outStream) override;

		bool PromptSaveFile(VirtualDirectory_t dirID, char *path, size_t &outPathLength, size_t pathCapacity, const PLPasStr &initialFileName, const PLPasStr &promptText) override;
		bool PromptOpenFile(VirtualDirectory_t dirID, char *path, size_t &outPathLength, size_t pathCapacity, const PLPasStr &promptText) override;

		static FileManagerImpl *GetInstance();

	private:
		typedef char ExtendedFileName_t[64 + 4];

		PLError_t OpenFileFork(VirtualDirectory_t dirID, const PLPasStr &filename, const char *ext, EFilePermission permission, GpIOStream *&outRefNum);
		PLError_t RawOpenFileFork(VirtualDirectory_t dirID, const PLPasStr &filename, const char *ext, EFilePermission permission, bool ignoreMeta, GpFileCreationDisposition_t createDisposition, GpIOStream *&outStream);

		static bool ConstructFilename(ExtendedFileName_t& extFN, const PLPasStr &fn, const char *extension);

		static FileManagerImpl ms_instance;
	};

	bool FileManagerImpl::FileExists(VirtualDirectory_t dirID, const PLPasStr &filename)
	{
		ExtendedFileName_t extFN;
		if (!ConstructFilename(extFN, filename, ".gpf"))
			return false;

		return HostFileSystem::GetInstance()->FileExists(dirID, extFN);
	}

	bool FileManagerImpl::FileLocked(VirtualDirectory_t dirID, const PLPasStr &filename)
	{
		const char *exts[3] = { ".gpf", ".gpa", ".gpd" };

		for (int extIndex = 0; extIndex < sizeof(exts) / sizeof(exts[0]); extIndex++)
		{
			ExtendedFileName_t extFN;
			if (!ConstructFilename(extFN, filename, exts[extIndex]))
				return true;

			bool exists = false;
			if (HostFileSystem::GetInstance()->FileLocked(dirID, extFN, &exists) && exists)
				return true;
		}

		return false;
	}

	bool FileManagerImpl::DeleteFile(VirtualDirectory_t dirID, const PLPasStr &filename)
	{
		const size_t numExts = 3;

		const char *exts[numExts] = { ".gpa", ".gpd", ".gpf" };
		const bool extMayNotExist[numExts] = { true, true, false };

		for (int extIndex = 0; extIndex < numExts; extIndex++)
		{
			ExtendedFileName_t extFN;
			if (!ConstructFilename(extFN, filename, exts[extIndex]))
				return true;

			bool existed = false;
			if (!PortabilityLayer::HostFileSystem::GetInstance()->DeleteFile(dirID, extFN, existed))
			{
				if (extMayNotExist[extIndex] && !existed)
					continue;
				else
					return false;
			}
		}

		return true;
	}

	PLError_t FileManagerImpl::CreateFile(VirtualDirectory_t dirID, const PLPasStr &filename, const MacFileProperties &mfp)
	{
		MacFilePropertiesSerialized serialized;
		serialized.Serialize(mfp);

		ExtendedFileName_t extFN;
		if (!ConstructFilename(extFN, filename, ".gpf"))
			return PLErrors::kBadFileName;

		GpIOStream *stream = nullptr;
		PLError_t err = RawOpenFileFork(dirID, filename, ".gpf", EFilePermission_Write, true, GpFileCreationDispositions::kCreateOrOverwrite, stream);
		if (err)
			return err;

		if (stream->Write(serialized.m_data, sizeof(serialized.m_data)) != sizeof(serialized.m_data))
		{
			stream->Close();
			return PLErrors::kIOError;
		}

		stream->Close();

		return PLErrors::kNone;
	}

	PLError_t FileManagerImpl::CreateFileAtCurrentTime(VirtualDirectory_t dirID, const PLPasStr &filename, const ResTypeID &fileCreator, const ResTypeID &fileType)
	{
		MacFileProperties mfp;
		fileCreator.ExportAsChars(mfp.m_fileCreator);
		fileType.ExportAsChars(mfp.m_fileType);
		mfp.m_creationDate = mfp.m_modifiedDate = PortabilityLayer::HostSystemServices::GetInstance()->GetTime();

		return CreateFile(dirID, filename, mfp);
	}

	PLError_t FileManagerImpl::OpenFileData(VirtualDirectory_t dirID, const PLPasStr &filename, EFilePermission permission, GpIOStream *&outStream)
	{
		return OpenFileFork(dirID, filename, ".gpd", permission, outStream);
	}

	PLError_t FileManagerImpl::OpenFileResources(VirtualDirectory_t dirID, const PLPasStr &filename, EFilePermission permission, GpIOStream *&outStream)
	{
		return OpenFileFork(dirID, filename, ".gpa", permission, outStream);
	}

	bool FileManagerImpl::ReadFileProperties(VirtualDirectory_t dirID, const PLPasStr &filename, MacFileProperties &properties)
	{
		GpIOStream *stream = nullptr;
		PLError_t err = RawOpenFileFork(dirID, filename, ".gpf", EFilePermission_Read, true, GpFileCreationDispositions::kOpenExisting, stream);
		if (err)
			return false;

		MacFilePropertiesSerialized serialized;
		bool readOk = (stream->Read(serialized.m_data, MacFilePropertiesSerialized::kSize) == MacFilePropertiesSerialized::kSize);
		stream->Close();

		if (readOk)
			serialized.Deserialize(properties);

		return readOk;
	}

	PLError_t FileManagerImpl::RawOpenFileData(VirtualDirectory_t dirID, const PLPasStr &filename, EFilePermission permission, bool ignoreMeta, GpFileCreationDisposition_t createDisposition, GpIOStream *&outStream)
	{
		return RawOpenFileFork(dirID, filename, ".gpd", permission, ignoreMeta, createDisposition, outStream);
	}

	PLError_t FileManagerImpl::RawOpenFileResources(VirtualDirectory_t dirID, const PLPasStr &filename, EFilePermission permission, bool ignoreMeta, GpFileCreationDisposition_t createDisposition, GpIOStream *&outStream)
	{
		return RawOpenFileFork(dirID, filename, ".gpa", permission, ignoreMeta, createDisposition, outStream);
	}

	bool FileManagerImpl::PromptSaveFile(VirtualDirectory_t dirID, char *path, size_t &outPathLength, size_t pathCapacity, const PLPasStr &initialFileName, const PLPasStr &promptText)
	{
		ExtendedFileName_t extFN;
		if (!ConstructFilename(extFN, initialFileName, ""))
			return false;

		return FileBrowserUI::Prompt(FileBrowserUI::Mode_Save, dirID, path, outPathLength, pathCapacity, initialFileName, promptText);
	}

	bool FileManagerImpl::PromptOpenFile(VirtualDirectory_t dirID, char *path, size_t &outPathLength, size_t pathCapacity, const PLPasStr &promptText)
	{
		return FileBrowserUI::Prompt(FileBrowserUI::Mode_Open, dirID, path, outPathLength, pathCapacity, PSTR(""), promptText);
	}

	FileManagerImpl *FileManagerImpl::GetInstance()
	{
		return &ms_instance;
	}

	PLError_t FileManagerImpl::OpenFileFork(VirtualDirectory_t dirID, const PLPasStr &filename, const char *extension, EFilePermission permission, GpIOStream *&outStream)
	{
		bool isWriteAccess = (permission == EFilePermission_Any || permission == EFilePermission_ReadWrite || permission == EFilePermission_Write);
		GpFileCreationDisposition_t createDisposition = isWriteAccess ? GpFileCreationDispositions::kCreateOrOpen : GpFileCreationDispositions::kOpenExisting;
		GpIOStream *stream = nullptr;
		PLError_t openError = RawOpenFileFork(dirID, filename, extension, permission, false, createDisposition, stream);
		if (openError != PLErrors::kNone)
			return openError;

		outStream = stream;

		return PLErrors::kNone;
	}

	PLError_t FileManagerImpl::RawOpenFileFork(VirtualDirectory_t dirID, const PLPasStr &filename, const char *ext, EFilePermission permission, bool ignoreMeta, GpFileCreationDisposition_t createDisposition, GpIOStream *&outStream)
	{
		ExtendedFileName_t gpfExtFN;
		ExtendedFileName_t extFN;

		if (filename.Length() > 63)
			return PLErrors::kBadFileName;

		if (!ignoreMeta)
		{
			if (!ConstructFilename(gpfExtFN, filename, ".gpf"))
				return PLErrors::kBadFileName;

			if (!HostFileSystem::GetInstance()->FileExists(dirID, gpfExtFN))
				return PLErrors::kFileNotFound;
		}

		if (!ConstructFilename(extFN, filename, ext))
			return PLErrors::kBadFileName;

		GpIOStream *fstream = nullptr;
		switch (permission)
		{
		case EFilePermission_Any:
			fstream = HostFileSystem::GetInstance()->OpenFile(dirID, extFN, true, createDisposition);
			if (fstream)
				permission = EFilePermission_ReadWrite;
			else
			{
				permission = EFilePermission_Read;
				fstream = HostFileSystem::GetInstance()->OpenFile(dirID, extFN, false, createDisposition);
			}
			break;
		case EFilePermission_Read:
			fstream = HostFileSystem::GetInstance()->OpenFile(dirID, extFN, false, createDisposition);
			break;
		case EFilePermission_ReadWrite:
		case EFilePermission_Write:
			fstream = HostFileSystem::GetInstance()->OpenFile(dirID, extFN, true, createDisposition);
			break;
		}

		if (!fstream)
			return PLErrors::kAccessDenied;

		outStream = fstream;

		return PLErrors::kNone;
	}

	bool FileManagerImpl::ConstructFilename(ExtendedFileName_t &extFN, const PLPasStr &fn, const char *extension)
	{
		const size_t fnameSize = fn.Length();
		if (fnameSize >= 64)
			return false;

		memcpy(extFN, fn.Chars(), fnameSize);
		memcpy(extFN + fnameSize, extension, strlen(extension) + 1);

		if (!PortabilityLayer::HostFileSystem::GetInstance()->ValidateFilePath(extFN, fnameSize))
			return false;

		return true;
	}

	FileManagerImpl FileManagerImpl::ms_instance;

	FileManager *FileManager::GetInstance()
	{
		return FileManagerImpl::GetInstance();
	}
}
