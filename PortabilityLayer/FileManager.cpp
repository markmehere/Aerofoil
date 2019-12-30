#include "FileManager.h"
#include "HostFileSystem.h"
#include "HostMemoryBuffer.h"
#include "MemReaderStream.h"
#include "MacBinary2.h"
#include "MacFileMem.h"
#include "PLPasStr.h"
#include "PLErrorCodes.h"
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
		bool DeleteFile(VirtualDirectory_t dirID, const PLPasStr &filename) override;

		PLError_t CreateFile(VirtualDirectory_t dirID, const PLPasStr &filename, const MacFileProperties &mfp) override;
		PLError_t CreateFileAtCurrentTime(VirtualDirectory_t dirID, const PLPasStr &filename, const ResTypeID &fileCreator, const ResTypeID &fileType) override;

		PLError_t OpenFileData(VirtualDirectory_t dirID, const PLPasStr &filename, EFilePermission filePermission, IOStream *&outRefNum) override;
		PLError_t OpenFileResources(VirtualDirectory_t dirID, const PLPasStr &filename, EFilePermission filePermission, IOStream *&outRefNum) override;
		bool ReadFileProperties(VirtualDirectory_t dirID, const PLPasStr &filename, MacFileProperties &properties) override;

		PLError_t RawOpenFileData(VirtualDirectory_t dirID, const PLPasStr &filename, EFilePermission filePermission, bool ignoreMeta, IOStream *&outStream) override;
		PLError_t RawOpenFileResources(VirtualDirectory_t dirID, const PLPasStr &filename, EFilePermission filePermission, bool ignoreMeta, IOStream *&outStream) override;

		static FileManagerImpl *GetInstance();

	private:
		typedef char ExtendedFileName_t[64 + 4];

		PLError_t OpenFileFork(VirtualDirectory_t dirID, const PLPasStr &filename, const char *ext, EFilePermission permission, IOStream *&outRefNum);
		PLError_t RawOpenFileFork(VirtualDirectory_t dirID, const PLPasStr &filename, const char *ext, EFilePermission permission, bool ignoreMeta, bool create, IOStream *&outStream);

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

	bool FileManagerImpl::DeleteFile(VirtualDirectory_t dirID, const PLPasStr &filename)
	{
		ExtendedFileName_t extFN;
		if (!ConstructFilename(extFN, filename, ".gpf"))
			return false;

		// PL_NotYetImplemented_TODO("FileSystem")
		return false;
	}

	PLError_t FileManagerImpl::CreateFile(VirtualDirectory_t dirID, const PLPasStr &filename, const MacFileProperties &mfp)
	{
		MacFilePropertiesSerialized serialized;
		serialized.Serialize(mfp);

		ExtendedFileName_t extFN;
		if (!ConstructFilename(extFN, filename, ".gpf"))
			return PLErrors::kBadFileName;

		IOStream *stream = nullptr;
		PLError_t err = RawOpenFileFork(dirID, filename, ".gpf", EFilePermission_Write, true, true, stream);
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

	PLError_t FileManagerImpl::OpenFileData(VirtualDirectory_t dirID, const PLPasStr &filename, EFilePermission permission, IOStream *&outStream)
	{
		return OpenFileFork(dirID, filename, ".gpd", permission, outStream);
	}

	PLError_t FileManagerImpl::OpenFileResources(VirtualDirectory_t dirID, const PLPasStr &filename, EFilePermission permission, IOStream *&outStream)
	{
		return OpenFileFork(dirID, filename, ".gpr", permission, outStream);
	}

	bool FileManagerImpl::ReadFileProperties(VirtualDirectory_t dirID, const PLPasStr &filename, MacFileProperties &properties)
	{
		IOStream *stream = nullptr;
		PLError_t err = RawOpenFileFork(dirID, filename, ".gpf", EFilePermission_Read, true, false, stream);
		if (err)
			return false;

		MacFilePropertiesSerialized serialized;
		bool readOk = (stream->Read(serialized.m_data, MacFilePropertiesSerialized::kSize) == MacFilePropertiesSerialized::kSize);
		stream->Close();

		if (readOk)
			serialized.Deserialize(properties);

		return readOk;
	}

	PLError_t FileManagerImpl::RawOpenFileData(VirtualDirectory_t dirID, const PLPasStr &filename, EFilePermission permission, bool ignoreMeta, IOStream *&outStream)
	{
		return RawOpenFileFork(dirID, filename, ".gpd", permission, ignoreMeta, false, outStream);
	}

	PLError_t FileManagerImpl::RawOpenFileResources(VirtualDirectory_t dirID, const PLPasStr &filename, EFilePermission permission, bool ignoreMeta, IOStream *&outStream)
	{
		return RawOpenFileFork(dirID, filename, ".gpr", permission, ignoreMeta, false, outStream);
	}

	FileManagerImpl *FileManagerImpl::GetInstance()
	{
		return &ms_instance;
	}

	PLError_t FileManagerImpl::OpenFileFork(VirtualDirectory_t dirID, const PLPasStr &filename, const char *extension, EFilePermission permission, IOStream *&outStream)
	{
		bool isWriteAccess = (permission == EFilePermission_Any || permission == EFilePermission_ReadWrite || permission == EFilePermission_Write);
		IOStream *stream = nullptr;
		PLError_t openError = RawOpenFileFork(dirID, filename, extension, permission, false, isWriteAccess, stream);
		if (openError != PLErrors::kNone)
			return openError;

		outStream = stream;

		return PLErrors::kNone;
	}

	PLError_t FileManagerImpl::RawOpenFileFork(VirtualDirectory_t dirID, const PLPasStr &filename, const char *ext, EFilePermission permission, bool ignoreMeta, bool create, IOStream *&outStream)
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

		const bool needToCreate = create && !HostFileSystem::GetInstance()->FileExists(dirID, extFN);

		IOStream *fstream = nullptr;
		switch (permission)
		{
		case EFilePermission_Any:
			fstream = HostFileSystem::GetInstance()->OpenFile(dirID, extFN, true, needToCreate);
			if (fstream)
				permission = EFilePermission_ReadWrite;
			else
			{
				permission = EFilePermission_Read;
				fstream = HostFileSystem::GetInstance()->OpenFile(dirID, extFN, false, needToCreate);
			}
			break;
		case EFilePermission_Read:
			fstream = HostFileSystem::GetInstance()->OpenFile(dirID, extFN, false, needToCreate);
			break;
		case EFilePermission_ReadWrite:
		case EFilePermission_Write:
			fstream = HostFileSystem::GetInstance()->OpenFile(dirID, extFN, true, needToCreate);
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

		for (size_t i = 0; i < fnameSize; i++)
		{
			const char c = extFN[i];
			if (c >= '0' && c <= '9')
				continue;

			if (c == '_' || c == '.' || c == '\'')
				continue;

			if (c == ' ' && i != 0 && i != fnameSize - 1)
				continue;

			if (c >= 'a' && c <= 'z')
				continue;

			if (c >= 'A' && c <= 'Z')
				continue;

			return false;
		}

		return true;
	}

	FileManagerImpl FileManagerImpl::ms_instance;

	FileManager *FileManager::GetInstance()
	{
		return FileManagerImpl::GetInstance();
	}
}
