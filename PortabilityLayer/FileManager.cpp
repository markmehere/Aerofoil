#include "FileManager.h"
#include "HostFileSystem.h"
#include "HostMemoryBuffer.h"
#include "MemReaderStream.h"
#include "MacBinary2.h"
#include "MacFileMem.h"
#include "PLPasStr.h"
#include "PLErrorCodes.h"

#include <vector>

namespace PortabilityLayer
{
	class VirtualFile;

	class FileManagerImpl final : public FileManager
	{
	public:
		bool FileExists(uint32_t dirID, const PLPasStr &filename) override;

		int OpenFileDF(uint32_t dirID, const PLPasStr &filename, EFilePermission filePermission, short *outRefNum) override;
		int OpenFileRF(uint32_t dirID, const PLPasStr &filename, EFilePermission filePermission, short *outRefNum) override;

		int RawOpenFileDF(uint32_t dirID, const PLPasStr &filename, EFilePermission filePermission, bool ignoreMeta, IOStream **outStream) override;
		int RawOpenFileRF(uint32_t dirID, const PLPasStr &filename, EFilePermission filePermission, bool ignoreMeta, IOStream **outStream) override;

		static FileManagerImpl *GetInstance();

	private:
		typedef char ExtendedFileName_t[64 + 4];

		struct OpenedFile
		{
			EVirtualDirectory m_dirID;
			PascalStr<64> m_fileName;

			IOStream *m_stream;
		};

		int OpenFileFork(uint32_t dirID, const PLPasStr &filename, const char *ext, EFilePermission permission, short *outRefNum);
		int RawOpenFileFork(uint32_t dirID, const PLPasStr &filename, const char *ext, EFilePermission permission, bool ignoreMeta, IOStream **outStream);

		static bool ConstructFilename(ExtendedFileName_t& extFN, const PLPasStr &fn, const char *extension);

		std::vector<OpenedFile> m_refs;

		static FileManagerImpl ms_instance;
	};

	bool FileManagerImpl::FileExists(uint32_t dirID, const PLPasStr &filename)
	{
		ExtendedFileName_t extFN;
		if (!ConstructFilename(extFN, filename, ".gpf"))
			return false;

		return HostFileSystem::GetInstance()->FileExists(static_cast<EVirtualDirectory>(dirID), extFN);
	}

	int FileManagerImpl::OpenFileDF(uint32_t dirID, const PLPasStr &filename, EFilePermission permission, short *outRefNum)
	{
		return OpenFileFork(dirID, filename, ".gpd", permission, outRefNum);
	}

	int FileManagerImpl::OpenFileRF(uint32_t dirID, const PLPasStr &filename, EFilePermission permission, short *outRefNum)
	{
		return OpenFileFork(dirID, filename, ".gpr", permission, outRefNum);
	}

	int FileManagerImpl::RawOpenFileDF(uint32_t dirID, const PLPasStr &filename, EFilePermission permission, bool ignoreMeta, IOStream **outStream)
	{
		return RawOpenFileFork(dirID, filename, ".gpd", permission, ignoreMeta, outStream);
	}

	int FileManagerImpl::RawOpenFileRF(uint32_t dirID, const PLPasStr &filename, EFilePermission permission, bool ignoreMeta, IOStream **outStream)
	{
		return RawOpenFileFork(dirID, filename, ".gpr", permission, ignoreMeta, outStream);
	}

	FileManagerImpl *FileManagerImpl::GetInstance()
	{
		return &ms_instance;
	}

	int FileManagerImpl::OpenFileFork(uint32_t dirID, const PLPasStr &filename, const char *extension, EFilePermission permission, short *outRefNum)
	{
		const size_t numRefs = m_refs.size();
		size_t refIndex = m_refs.size();
		for (size_t i = 0; i < numRefs; i++)
		{
			if (m_refs[i].m_stream == nullptr)
			{
				refIndex = i;
				break;
			}
		}

		if (refIndex == 0x7fff)
			return tmfoErr;

		IOStream *stream = nullptr;
		int openError = RawOpenFileFork(dirID, filename, extension, permission, false, &stream);
		if (openError != 0)
			return openError;

		if (refIndex == numRefs)
			m_refs.push_back(OpenedFile());

		OpenedFile &of = m_refs[refIndex];
		of.m_stream = stream;
		of.m_dirID = static_cast<EVirtualDirectory>(dirID);
		of.m_fileName.Set(filename.Length(), filename.Chars());

		*outRefNum = static_cast<short>(refIndex + 1);

		return noErr;
	}

	int FileManagerImpl::RawOpenFileFork(uint32_t dirID, const PLPasStr &filename, const char *ext, EFilePermission permission, bool ignoreMeta, IOStream **outStream)
	{
		ExtendedFileName_t gpfExtFN;
		ExtendedFileName_t extFN;

		if (filename.Length() > 63)
			return bdNamErr;

		if (!ignoreMeta)
		{
			if (!ConstructFilename(gpfExtFN, filename, ".gpf"))
				return bdNamErr;

			if (!HostFileSystem::GetInstance()->FileExists(static_cast<EVirtualDirectory>(dirID), gpfExtFN))
				return fnfErr;
		}

		const bool needToCreate = !(ignoreMeta || HostFileSystem::GetInstance()->FileExists(static_cast<EVirtualDirectory>(dirID), extFN));

		if (!ConstructFilename(extFN, filename, ext))
			return bdNamErr;

		IOStream *fstream = nullptr;
		switch (permission)
		{
		case EFilePermission_Any:
			fstream = HostFileSystem::GetInstance()->OpenFile(static_cast<EVirtualDirectory>(dirID), extFN, true, needToCreate);
			if (fstream)
				permission = EFilePermission_ReadWrite;
			else
			{
				permission = EFilePermission_Read;
				fstream = HostFileSystem::GetInstance()->OpenFile(static_cast<EVirtualDirectory>(dirID), extFN, false, needToCreate);
			}
			break;
		case EFilePermission_Read:
			fstream = HostFileSystem::GetInstance()->OpenFile(static_cast<EVirtualDirectory>(dirID), extFN, false, needToCreate);
			break;
		case EFilePermission_ReadWrite:
			fstream = HostFileSystem::GetInstance()->OpenFile(static_cast<EVirtualDirectory>(dirID), extFN, true, needToCreate);
			break;
		}

		if (!fstream)
			return permErr;

		*outStream = fstream;
		return noErr;
	}

	bool FileManagerImpl::ConstructFilename(ExtendedFileName_t& extFN, const PLPasStr &fn, const char *extension)
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

			if (c == '_')
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
