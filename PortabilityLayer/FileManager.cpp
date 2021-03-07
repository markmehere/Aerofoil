#include "FileManager.h"

#include "CombinedTimestamp.h"
#include "FileBrowserUI.h"
#include "IGpFileSystem.h"
#include "IGpSystemServices.h"
#include "MemReaderStream.h"
#include "MacBinary2.h"
#include "MacFileMem.h"
#include "ResTypeID.h"
#include "ZipFileProxy.h"

#include "PLDrivers.h"
#include "PLPasStr.h"
#include "PLErrorCodes.h"
#include "PLSysCalls.h"

#include <vector>

namespace PortabilityLayer
{
	class VirtualFile;
	class CompositeFileImpl;

	typedef char ExtendedFileName_t[64 + 4];

	namespace FileManagerTools
	{
		bool ConstructFilename(ExtendedFileName_t& extFN, const PLPasStr &fn, const char *extension);
	};

	class FileManagerImpl final : public FileManager
	{
	public:
		CompositeFile *OpenCompositeFile(VirtualDirectory_t dirID, const PLPasStr &filename) override;
		PLError_t OpenNonCompositeFile(VirtualDirectory_t dirID, const PLPasStr &filename, const char *extension, EFilePermission filePermission, GpFileCreationDisposition_t creationDisposition, GpIOStream *&outStream) override;

		bool FileExists(VirtualDirectory_t dirID, const PLPasStr &filename) override;

		bool DeleteNonCompositeFile(VirtualDirectory_t dirID, const PLPasStr &filename, const char *ext) override;
		bool DeleteCompositeFile(VirtualDirectory_t dirID, const PLPasStr &filename) override;

		PLError_t CreateFile(VirtualDirectory_t dirID, const PLPasStr &filename, const MacFileProperties &mfp) override;
		PLError_t CreateFileAtCurrentTime(VirtualDirectory_t dirID, const PLPasStr &filename, const ResTypeID &fileCreator, const ResTypeID &fileType) override;

		PLError_t RawOpenFileData(VirtualDirectory_t dirID, const PLPasStr &filename, EFilePermission filePermission, bool ignoreMeta, GpFileCreationDisposition_t creationDisposition, GpIOStream *&outStream) override;
		PLError_t RawOpenFileResources(VirtualDirectory_t dirID, const PLPasStr &filename, EFilePermission filePermission, bool ignoreMeta, GpFileCreationDisposition_t creationDisposition, GpIOStream *&outStream) override;

		bool PromptSaveFile(VirtualDirectory_t dirID, const char *extension, char *path, size_t &outPathLength, size_t pathCapacity, const PLPasStr &initialFileName, const PLPasStr &promptText, bool composites, const FileBrowserUI_DetailsCallbackAPI &callbackAPI) override;
		bool PromptOpenFile(VirtualDirectory_t dirID, const char *extension, char *path, size_t &outPathLength, size_t pathCapacity, const PLPasStr &promptText, bool composites, const FileBrowserUI_DetailsCallbackAPI &callbackAPI) override;

		static FileManagerImpl *GetInstance();

	private:
		PLError_t OpenFileFork(VirtualDirectory_t dirID, const PLPasStr &filename, const char *ext, EFilePermission permission, GpIOStream *&outRefNum);
		PLError_t RawOpenFileFork(VirtualDirectory_t dirID, const PLPasStr &filename, const char *ext, EFilePermission permission, bool ignoreMeta, GpFileCreationDisposition_t createDisposition, GpIOStream *&outStream);

		static FileManagerImpl ms_instance;
	};

	class CompositeFileImpl final : public CompositeFile
	{
	public:
		PLError_t OpenData(EFilePermission filePermission, GpFileCreationDisposition_t disposition, GpIOStream *&outStream) override;
		PLError_t OpenResources(GpIOStream *&outStream, ZipFileProxy *&outProxy, bool &outIsProxyShared) override;
		const MacFileProperties &GetProperties() const override;

		bool IsDataReadOnly() const override;

		void Close() override;

		static CompositeFileImpl *Create(VirtualDirectory_t dirID, const PLPasStr &filename, GpIOStream *stream, ZipFileProxy *zipFile, const MacFileProperties &mfp, bool resInline, bool dataInline, size_t inlineDataIndex);

	private:
		CompositeFileImpl(VirtualDirectory_t dirID, const PLPasStr &filename, GpIOStream *stream, ZipFileProxy *zipFile, const MacFileProperties &mfp, bool resInline, bool dataInline, size_t inlineDataIndex);
		~CompositeFileImpl();

		VirtualDirectory_t m_dirID;
		PascalStr<255> m_filename;
		GpIOStream *m_stream;
		ZipFileProxy *m_zipFile;
		MacFileProperties m_mfp;
		size_t m_inlineDataIndex;
		bool m_resInline;
		bool m_dataInline;
	};


	CompositeFile *FileManagerImpl::OpenCompositeFile(VirtualDirectory_t dirID, const PLPasStr &filename)
	{
		ExtendedFileName_t extFN;
		if (!FileManagerTools::ConstructFilename(extFN, filename, ".gpf"))
			return nullptr;

		GpIOStream *stream = PLDrivers::GetFileSystem()->OpenFile(dirID, extFN, false, GpFileCreationDispositions::kOpenExisting);
		if (!stream)
			return nullptr;

		ZipFileProxy *zipFile = ZipFileProxy::Create(stream);
		if (!zipFile)
		{
			stream->Close();
			return nullptr;
		}

		size_t metaIndex = 0;
		if (!zipFile->IndexFile("!!meta", metaIndex))
		{
			stream->Close();
			return nullptr;
		}

		MacFilePropertiesSerialized mfps;

		GpIOStream *metaStream = zipFile->OpenFile(metaIndex);
		if (!metaStream)
		{
			zipFile->Destroy();
			stream->Close();
			return nullptr;
		}

		if (!metaStream->ReadExact(mfps.m_data, sizeof(mfps.m_data)))
		{
			metaStream->Close();
			zipFile->Destroy();
			stream->Close();
			return nullptr;
		}

		metaStream->Close();

		MacFileProperties mfp;
		mfps.Deserialize(mfp);

		size_t dataIndex = 0;
		bool hasData = zipFile->IndexFile("!data", dataIndex);

		size_t nonResFiles = 1 + (hasData ? 1 : 0);
		bool hasResources = (zipFile->NumFiles() > nonResFiles);

		if (!hasData && !hasResources)
		{
			zipFile->Destroy();
			zipFile = nullptr;

			stream->Close();
			stream = nullptr;
		}

		CompositeFile *compositeFile = CompositeFileImpl::Create(dirID, filename, stream, zipFile, mfp, hasResources, hasData, hasData ? dataIndex : 0);
		if (!compositeFile)
		{
			if (zipFile)
				zipFile->Destroy();

			if (stream)
				stream->Close();

			return nullptr;
		}

		return compositeFile;
	}

	PLError_t FileManagerImpl::OpenNonCompositeFile(VirtualDirectory_t dirID, const PLPasStr &filename, const char *extension, EFilePermission filePermission, GpFileCreationDisposition_t creationDisposition, GpIOStream *&outStream)
	{
		return RawOpenFileFork(dirID, filename, extension, filePermission, true, creationDisposition, outStream);
	}

	bool FileManagerImpl::FileExists(VirtualDirectory_t dirID, const PLPasStr &filename)
	{
		ExtendedFileName_t extFN;
		if (!FileManagerTools::ConstructFilename(extFN, filename, ".gpf"))
			return false;

		return PLDrivers::GetFileSystem()->FileExists(dirID, extFN);
	}

	bool FileManagerImpl::DeleteNonCompositeFile(VirtualDirectory_t dirID, const PLPasStr &filename, const char *ext)
	{
		ExtendedFileName_t extFN;
		if (!FileManagerTools::ConstructFilename(extFN, filename, ext))
			return true;

		bool existed = false;
		if (!PLDrivers::GetFileSystem()->DeleteFile(dirID, extFN, existed))
		{
			if (!existed)
				return false;
		}

		return true;
	}

	bool FileManagerImpl::DeleteCompositeFile(VirtualDirectory_t dirID, const PLPasStr &filename)
	{
		const size_t numExts = 3;

		const char *exts[numExts] = { ".gpa", ".gpd", ".gpf" };
		const bool extMayNotExist[numExts] = { true, true, false };

		for (int extIndex = 0; extIndex < numExts; extIndex++)
		{
			ExtendedFileName_t extFN;
			if (!FileManagerTools::ConstructFilename(extFN, filename, exts[extIndex]))
				return true;

			bool existed = false;
			if (!PLDrivers::GetFileSystem()->DeleteFile(dirID, extFN, existed))
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
		if (!FileManagerTools::ConstructFilename(extFN, filename, ".gpf"))
			return PLErrors::kBadFileName;

		GpIOStream *stream = nullptr;
		PLError_t err = RawOpenFileFork(dirID, filename, ".gpf", EFilePermission_Write, true, GpFileCreationDispositions::kCreateOrOverwrite, stream);
		if (err)
			return err;

		CombinedTimestamp ts;

		if (!serialized.WriteAsPackage(*stream, ts))
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
		mfp.m_createdTimeMacEpoch = mfp.m_modifiedTimeMacEpoch = PLDrivers::GetSystemServices()->GetTime();

		return CreateFile(dirID, filename, mfp);
	}

	PLError_t FileManagerImpl::RawOpenFileData(VirtualDirectory_t dirID, const PLPasStr &filename, EFilePermission permission, bool ignoreMeta, GpFileCreationDisposition_t createDisposition, GpIOStream *&outStream)
	{
		return RawOpenFileFork(dirID, filename, ".gpd", permission, ignoreMeta, createDisposition, outStream);
	}

	PLError_t FileManagerImpl::RawOpenFileResources(VirtualDirectory_t dirID, const PLPasStr &filename, EFilePermission permission, bool ignoreMeta, GpFileCreationDisposition_t createDisposition, GpIOStream *&outStream)
	{
		return RawOpenFileFork(dirID, filename, ".gpa", permission, ignoreMeta, createDisposition, outStream);
	}

	bool FileManagerImpl::PromptSaveFile(VirtualDirectory_t dirID, const char *extension, char *path, size_t &outPathLength, size_t pathCapacity, const PLPasStr &initialFileName, const PLPasStr &promptText, bool composites, const FileBrowserUI_DetailsCallbackAPI &detailsAPI)
	{
		ExtendedFileName_t extFN;
		if (!FileManagerTools::ConstructFilename(extFN, initialFileName, ""))
			return false;

		return FileBrowserUI::Prompt(FileBrowserUI::Mode_Save, dirID, extension, path, outPathLength, pathCapacity, initialFileName, promptText, composites, detailsAPI);
	}

	bool FileManagerImpl::PromptOpenFile(VirtualDirectory_t dirID, const char *extension, char *path, size_t &outPathLength, size_t pathCapacity, const PLPasStr &promptText, bool composites, const FileBrowserUI_DetailsCallbackAPI &detailsAPI)
	{
		return FileBrowserUI::Prompt(FileBrowserUI::Mode_Open, dirID, extension, path, outPathLength, pathCapacity, PSTR(""), promptText, composites, detailsAPI);
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
			if (!FileManagerTools::ConstructFilename(gpfExtFN, filename, ".gpf"))
				return PLErrors::kBadFileName;

			if (!PLDrivers::GetFileSystem()->FileExists(dirID, gpfExtFN))
				return PLErrors::kFileNotFound;
		}

		if (!FileManagerTools::ConstructFilename(extFN, filename, ext))
			return PLErrors::kBadFileName;

		GpIOStream *fstream = nullptr;
		switch (permission)
		{
		case EFilePermission_Any:
			fstream = PLDrivers::GetFileSystem()->OpenFile(dirID, extFN, true, createDisposition);
			if (fstream)
				permission = EFilePermission_ReadWrite;
			else
			{
				permission = EFilePermission_Read;
				fstream = PLDrivers::GetFileSystem()->OpenFile(dirID, extFN, false, createDisposition);
			}
			break;
		case EFilePermission_Read:
			fstream = PLDrivers::GetFileSystem()->OpenFile(dirID, extFN, false, createDisposition);
			break;
		case EFilePermission_ReadWrite:
		case EFilePermission_Write:
			fstream = PLDrivers::GetFileSystem()->OpenFile(dirID, extFN, true, createDisposition);
			break;
		}

		if (!fstream)
		{
			if (ignoreMeta)
			{
				if (!PLDrivers::GetFileSystem()->FileExists(dirID, extFN))
					return PLErrors::kFileNotFound;
			}

			return PLErrors::kAccessDenied;
		}

		outStream = fstream;

		return PLErrors::kNone;
	}

	FileManagerImpl FileManagerImpl::ms_instance;

	FileManager *FileManager::GetInstance()
	{
		return FileManagerImpl::GetInstance();
	}


	bool FileManagerTools::ConstructFilename(ExtendedFileName_t &extFN, const PLPasStr &fn, const char *extension)
	{
		const size_t fnameSize = fn.Length();
		if (fnameSize >= 64)
			return false;

		memcpy(extFN, fn.Chars(), fnameSize);
		memcpy(extFN + fnameSize, extension, strlen(extension) + 1);

		if (!PLDrivers::GetFileSystem()->ValidateFilePath(extFN, fnameSize))
			return false;

		return true;
	}


	// ==========================================================================
	PLError_t CompositeFileImpl::OpenData(EFilePermission filePermission, GpFileCreationDisposition_t disposition, GpIOStream *&outStream)
	{
		if (m_dataInline)
		{
			if (filePermission == EFilePermission_Any || filePermission == EFilePermission_Read)
			{
				GpIOStream *stream = m_zipFile->OpenFile(m_inlineDataIndex);
				if (!stream)
					return PLErrors::kIOError;

				outStream = stream;
				return PLErrors::kNone;
			}
			else
				return PLErrors::kAccessDenied;
		}
		else
			return FileManager::GetInstance()->RawOpenFileData(m_dirID, m_filename.ToShortStr(), filePermission, true, disposition, outStream);
	}

	PLError_t CompositeFileImpl::OpenResources(GpIOStream *&outStream, ZipFileProxy *&outProxy, bool &outIsProxyShared)
	{
		if (m_resInline)
		{
			outStream = nullptr;
			outProxy = m_zipFile;
			outIsProxyShared = true;
			return PLErrors::kNone;
		}
		else
		{
			GpIOStream *stream = nullptr;
			PLError_t err = FileManager::GetInstance()->RawOpenFileResources(m_dirID, m_filename.ToShortStr(), EFilePermission_Read, true, GpFileCreationDispositions::kOpenExisting, stream);
			if (err != PLErrors::kNone)
				return err;

			ZipFileProxy *proxy = ZipFileProxy::Create(stream);
			if (!proxy)
			{
				stream->Close();
				return PLErrors::kIOError;
			}

			outStream = stream;
			outProxy = proxy;
			outIsProxyShared = false;
			return PLErrors::kNone;
		}
	}

	const MacFileProperties &CompositeFileImpl::GetProperties() const
	{
		return m_mfp;
	}

	bool CompositeFileImpl::IsDataReadOnly() const
	{
		if (m_dataInline)
			return true;

		ExtendedFileName_t extFN;
		if (!FileManagerTools::ConstructFilename(extFN, m_filename.ToShortStr(), ".gpd"))
			return false;

		bool exists = false;
		return PLDrivers::GetFileSystem()->FileLocked(m_dirID, extFN, exists);
	}

	void CompositeFileImpl::Close()
	{
		this->~CompositeFileImpl();
		free(this);
	}

	CompositeFileImpl *CompositeFileImpl::Create(VirtualDirectory_t dirID, const PLPasStr &filename, GpIOStream *stream, ZipFileProxy *zipFile, const MacFileProperties &mfp, bool resInline, bool dataInline, size_t inlineDataIndex)
	{
		void *storage = malloc(sizeof(CompositeFileImpl));
		if (!storage)
			return nullptr;

		return new (storage) CompositeFileImpl(dirID, filename, stream, zipFile, mfp, resInline, dataInline, inlineDataIndex);
	}

	CompositeFileImpl::CompositeFileImpl(VirtualDirectory_t dirID, const PLPasStr &filename, GpIOStream *stream, ZipFileProxy *zipFile, const MacFileProperties &mfp, bool resInline, bool dataInline, size_t inlineDataIndex)
		: m_dirID(dirID)
		, m_filename(filename)
		, m_stream(stream)
		, m_zipFile(zipFile)
		, m_mfp(mfp)
		, m_resInline(resInline)
		, m_dataInline(dataInline)
		, m_inlineDataIndex(inlineDataIndex)
	{
	}

	CompositeFileImpl::~CompositeFileImpl()
	{
		if (m_zipFile)
			m_zipFile->Destroy();

		if (m_stream)
			m_stream->Close();
	}
}
