#include "ResourceManager.h"

#include "BinarySearch.h"
#include "BMPFormat.h"
#include "FileManager.h"
#include "GPArchive.h"
#include "HostDirectoryCursor.h"
#include "HostFileSystem.h"
#include "HostMemoryBuffer.h"
#include "GpIOStream.h"
#include "MacBinary2.h"
#include "MacFileMem.h"
#include "MemReaderStream.h"
#include "MemoryManager.h"
#include "MMHandleBlock.h"
#include "ResourceCompiledTypeList.h"
#include "ResourceFile.h"
#include "PLPasStr.h"
#include "PLErrorCodes.h"
#include "VirtualDirectory.h"
#include "WaveFormat.h"
#include "ZipFileProxy.h"
#include "ZipFile.h"

#include <stdio.h>
#include <algorithm>

namespace ResourceValidationRules
{
	enum ResourceValidationRule
	{
		kNone,
		kWAV,
		kBMP,
	};
}

static const char *kPICTExtension = ".bmp";

typedef ResourceValidationRules::ResourceValidationRule ResourceValidationRule_t;

namespace
{
	// Validation here is only intended to be minimal, to ensure later checks can determine the format size and do certain operations
	// that must be valid.
	static bool ValidateResource(const void *res, size_t size, ResourceValidationRule_t validationRule)
	{
		switch (validationRule)
		{
		case ResourceValidationRules::kWAV:
			{
				if (size < sizeof(PortabilityLayer::RIFFTag))
					return false;

				PortabilityLayer::RIFFTag mainTag;
				memcpy(&mainTag, res, sizeof(mainTag));
				if (mainTag.m_chunkSize > size - sizeof(sizeof(PortabilityLayer::RIFFTag)))
					return false;

				return true;
			}
			break;

		case ResourceValidationRules::kBMP:
			{
				if (size < sizeof(PortabilityLayer::BitmapFileHeader) + sizeof(PortabilityLayer::BitmapInfoHeader))
					return false;

				PortabilityLayer::BitmapFileHeader mainHeader;
				memcpy(&mainHeader, res, sizeof(mainHeader));
				if (mainHeader.m_fileSize > size)
					return false;

				PortabilityLayer::BitmapInfoHeader infoHeader;
				memcpy(&infoHeader, static_cast<const uint8_t*>(res) + sizeof(mainHeader), sizeof(infoHeader));
				if (infoHeader.m_thisStructureSize < sizeof(PortabilityLayer::BitmapInfoHeader))
					return false;

				const size_t sizeForInfoHeader = size - sizeof(PortabilityLayer::BitmapFileHeader);
				if (infoHeader.m_thisStructureSize > sizeForInfoHeader)
					return false;

				// Dimensions need to fit in 16-bit signed space
				if (infoHeader.m_width >= 0x8000 || infoHeader.m_height >= 0x8000)
					return false;

				return true;
			}
			break;

		default:
			break;
		};

		return false;
	}
}


namespace PortabilityLayer
{
	struct MMHandleBlock;
	struct IResourceArchive;

	class ResourceManagerImpl final : public ResourceManager
	{
	public:
		ResourceManagerImpl();

		void Init() override;
		void Shutdown() override;

		THandle<void> GetAppResource(const ResTypeID &resTypeID, int16_t resID) const override;
		IResourceArchive *GetAppResourceArchive() const override;

		IResourceArchive *LoadResFile(VirtualDirectory_t virtualDir, const PLPasStr &filename) const override;
		PLError_t CreateBlankResFile(VirtualDirectory_t virtualDir, const PLPasStr &filename) override;

		void DissociateHandle(MMHandleBlock *hdl) const override;
		const ResourceArchiveRef *ResourceForHandle(MMHandleBlock *hdl) const override;

		static ResourceManagerImpl *GetInstance();

	private:
		void UnloadAndDestroyResourceFile(IResourceArchive *rf);

		IResourceArchive *LoadResDirectory(VirtualDirectory_t virtualDir, const PLPasStr &filename) const;

		IResourceArchive *m_appResArchive;

		static ResourceManagerImpl ms_instance;
	};

	ResourceManagerImpl::ResourceManagerImpl()
		: m_appResArchive(nullptr)
	{
	}

	void ResourceManagerImpl::Init()
	{
		m_appResArchive = LoadResFile(VirtualDirectories::kApplicationData, PSTR("ApplicationResources"));
	}

	void ResourceManagerImpl::Shutdown()
	{
		if (m_appResArchive)
			m_appResArchive->Destroy();

		m_appResArchive = nullptr;
	}

	void ResourceManagerImpl::DissociateHandle(MMHandleBlock *hdl) const
	{
		assert(hdl->m_rmSelfRef);
		assert(hdl->m_rmSelfRef->m_handle == hdl);
		hdl->m_rmSelfRef->m_handle = nullptr;
		hdl->m_rmSelfRef = nullptr;
	}

	const ResourceArchiveRef *ResourceManagerImpl::ResourceForHandle(MMHandleBlock *hdl) const
	{
		return hdl->m_rmSelfRef;
	}

	void ResourceManagerImpl::UnloadAndDestroyResourceFile(IResourceArchive *rf)
	{
		rf->Destroy();
	}

	IResourceArchive *ResourceManagerImpl::LoadResDirectory(VirtualDirectory_t virtualDir, const PLPasStr &filename) const
	{
		ResourceArchiveDirectory *archive = ResourceArchiveDirectory::Create(virtualDir, filename);
		if (!archive)
			return nullptr;

		if (!archive->Init())
		{
			archive->Destroy();
			return nullptr;
		}

		return archive;
	}

	ResourceManagerImpl *ResourceManagerImpl::GetInstance()
	{
		return &ms_instance;
	}

	IResourceArchive *ResourceManagerImpl::LoadResFile(VirtualDirectory_t virtualDir, const PLPasStr &filename) const
	{
		if (PortabilityLayer::HostFileSystem::GetInstance()->IsVirtualDirectoryLooseResources(virtualDir))
			return LoadResDirectory(virtualDir, filename);

		GpIOStream *fStream = nullptr;
		if (FileManager::GetInstance()->RawOpenFileResources(virtualDir, filename, EFilePermission_Read, true, GpFileCreationDispositions::kOpenExisting, fStream) != PLErrors::kNone)
			return nullptr;

		ZipFileProxy *proxy = ZipFileProxy::Create(fStream);
		if (!proxy)
		{
			fStream->Close();
			return nullptr;
		}

		IResourceArchive *archive = ResourceArchiveZipFile::Create(proxy, fStream);
		if (!archive)
		{
			proxy->Destroy();
			fStream->Close();
			return nullptr;
		}

		return archive;
	}

	PLError_t ResourceManagerImpl::CreateBlankResFile(VirtualDirectory_t virtualDir, const PLPasStr &filename)
	{
		GpIOStream *stream = nullptr;
		PLError_t error = FileManager::GetInstance()->RawOpenFileResources(virtualDir, filename, EFilePermission_Write, true, GpFileCreationDispositions::kCreateOrOverwrite, stream);
		if (error)
			return error;

		PortabilityLayer::ZipEndOfCentralDirectoryRecord eocd;
		eocd.m_signature = PortabilityLayer::ZipEndOfCentralDirectoryRecord::kSignature;
		eocd.m_thisDiskNumber = 0;
		eocd.m_centralDirDisk = 0;
		eocd.m_numCentralDirRecordsThisDisk = 0;
		eocd.m_numCentralDirRecords = 0;
		eocd.m_centralDirectorySizeBytes = 0;
		eocd.m_centralDirStartOffset = 0;
		eocd.m_commentLength = 0;

		if (stream->Write(&eocd, sizeof(eocd)) != sizeof(eocd))
		{
			stream->Close();
			return PLErrors::kIOError;
		}

		stream->Close();

		return PLErrors::kNone;
	}

	THandle<void> ResourceManagerImpl::GetAppResource(const ResTypeID &resType, int16_t resID) const
	{
		if (!m_appResArchive)
			return THandle<void>();

		return m_appResArchive->LoadResource(resType, resID);
	}

	IResourceArchive *ResourceManagerImpl::GetAppResourceArchive() const
	{
		return m_appResArchive;
	}

	ResourceManagerImpl ResourceManagerImpl::ms_instance;

	ResourceManager *ResourceManager::GetInstance() { return ResourceManagerImpl::GetInstance(); }

	// ===========================================================================================

	const char *ResourceArchiveBase::GetFileExtensionForResType(const ResTypeID &resTypeID, int &outValidationRule)
	{
		const char *extension = ".bin";
		outValidationRule = ResourceValidationRules::kNone;

		if (resTypeID == ResTypeID('snd '))
		{
			extension = ".wav";
			outValidationRule = ResourceValidationRules::kWAV;
		}
		else if (resTypeID == ResTypeID('Date') || resTypeID == ResTypeID('PICT'))
		{
			extension = kPICTExtension;
			outValidationRule = ResourceValidationRules::kBMP;
		}
		else if (resTypeID == ResTypeID('STR#'))
			extension = ".txt";
		else if (resTypeID == ResTypeID('DITL') || resTypeID == ResTypeID('muvi'))
			extension = ".json";

		return extension;
	}

	// ===========================================================================================

	ResourceArchiveRef::ResourceArchiveRef()
		: m_handle(nullptr)
		, m_size(0)
		, m_resID(0)
	{
	}

	// ===========================================================================================

	ResourceArchiveZipFile *ResourceArchiveZipFile::Create(ZipFileProxy *zipFileProxy, GpIOStream *stream)
	{
		PortabilityLayer::MemoryManager *mm = PortabilityLayer::MemoryManager::GetInstance();

		size_t numFiles = zipFileProxy->NumFiles();

		ResourceArchiveRef *refs = nullptr;
		if (numFiles > 0)
		{
			refs = static_cast<ResourceArchiveRef*>(mm->Alloc(sizeof(ResourceArchiveRef) * numFiles));
			if (!refs)
				return nullptr;

			for (size_t i = 0; i < numFiles; i++)
				new (refs + i) ResourceArchiveRef();
		}

		void *storage = mm->Alloc(sizeof(ResourceArchiveZipFile));
		if (!storage)
		{
			mm->Release(refs);
			return nullptr;
		}

		return new (storage) ResourceArchiveZipFile(zipFileProxy, stream, refs);
	}

	void ResourceArchiveZipFile::Destroy()
	{
		this->~ResourceArchiveZipFile();
		PortabilityLayer::MemoryManager::GetInstance()->Release(this);
	}

	THandle<void> ResourceArchiveZipFile::LoadResource(const ResTypeID &resTypeID, int id)
	{
		return GetResource(resTypeID, id, true);
	}

	bool ResourceArchiveZipFile::HasAnyResourcesOfType(const ResTypeID &resTypeID) const
	{
		char resPrefix[6];
		resTypeID.ExportAsChars(resPrefix);
		resPrefix[4] = '/';
		resPrefix[5] = '\0';

		return m_zipFileProxy->HasPrefix(resPrefix);
	}

	bool ResourceArchiveZipFile::FindFirstResourceOfType(const ResTypeID &resTypeID, int16_t &outID) const
	{
		char resPrefix[6];
		resTypeID.ExportAsChars(resPrefix);
		resPrefix[4] = '/';
		resPrefix[5] = '\0';

		size_t firstFileIndex = 0;
		if (!m_zipFileProxy->FindFirstWithPrefix(resPrefix, firstFileIndex))
			return false;

		const size_t numFiles = m_zipFileProxy->NumFiles();

		bool haveAny = false;
		int16_t lowestID = 32767;

		for (size_t fileIndex = firstFileIndex; fileIndex < numFiles; fileIndex++)
		{
			const char *resName = nullptr;
			size_t fnLength = 0;
			m_zipFileProxy->GetFileName(fileIndex, resName, fnLength);

			if (fnLength <= 5 || memcmp(resName, resPrefix, 5))
				break;

			const char *idChars = resName + 5;
			size_t idCharsRemaining = fnLength - 5;

			const size_t extLength = strlen(kPICTExtension);

			if (idCharsRemaining <= extLength)
				continue;

			if (memcmp(idChars + idCharsRemaining - extLength, kPICTExtension, extLength))
				continue;

			idCharsRemaining -= extLength;

			bool isNegative = false;

			if (idChars[0] == '-')
			{
				isNegative = true;
				idCharsRemaining--;
				idChars++;
			}

			if (idCharsRemaining == 0)
				continue;

			if (idChars[0] == '0' && (idCharsRemaining > 1 || isNegative))
				continue;

			int32_t resID = 0;
			bool failedName = false;
			while (idCharsRemaining)
			{
				const char idChar = *idChars;

				if (idChar < '0' || idChar > '9')
				{
					failedName = true;
					break;
				}

				resID = resID * 10;
				if (isNegative)
				{
					resID -= (idChar - '0');
					if (resID < -32768)
					{
						failedName = true;
						break;
					}
				}
				else
				{
					resID += (idChar - '0');
					if (resID > 32767)
					{
						failedName = true;
						break;
					}
				}

				idChars++;
				idCharsRemaining--;
			}

			if (failedName)
				continue;

			if (haveAny == false || resID < lowestID)
			{
				lowestID = resID;
				haveAny = true;
			}
		}

		if (haveAny)
			outID = static_cast<int16_t>(lowestID);

		return haveAny;
	}

	bool ResourceArchiveZipFile::IndexResource(const ResTypeID &resTypeID, int id, size_t &outIndex, int &outValidationRule) const
	{
		const char *extension = GetFileExtensionForResType(resTypeID, outValidationRule);

		char resourceFile[64];

		GpArcResourceTypeTag resTag = GpArcResourceTypeTag::Encode(resTypeID);

		snprintf(resourceFile, sizeof(resourceFile) - 1, "%s/%i%s", resTag.m_id, id, extension);

		size_t index = 0;
		return m_zipFileProxy->IndexFile(resourceFile, outIndex);
	}

	THandle<void> ResourceArchiveZipFile::GetResource(const ResTypeID &resTypeID, int id, bool load)
	{
		int validationRule = 0;
		size_t index = 0;
		if (!IndexResource(resTypeID, id, index, validationRule))
			return THandle<void>();

		ResourceArchiveRef *ref = m_resourceHandles + index;

		MMHandleBlock *handle = nullptr;
		if (ref->m_handle != nullptr)
			handle = ref->m_handle;
		else
		{
			handle = MemoryManager::GetInstance()->AllocHandle(0);
			if (!handle)
				return THandle<void>();

			handle->m_rmSelfRef = ref;
			ref->m_handle = handle;
			ref->m_resID = static_cast<int16_t>(id);
			ref->m_size = m_zipFileProxy->GetFileSize(index);
		}

		if (handle->m_contents == nullptr && load)
		{
			if (ref->m_size > 0)
			{
				void *contents = MemoryManager::GetInstance()->Alloc(ref->m_size);
				handle->m_contents = contents;
				handle->m_size = ref->m_size;

				if (!m_zipFileProxy->LoadFile(index, contents) || (validationRule != ResourceValidationRules::kNone && !ValidateResource(contents, ref->m_size, static_cast<ResourceValidationRule_t>(validationRule))))
				{
					MemoryManager::GetInstance()->Release(contents);
					handle->m_contents = nullptr;
					handle->m_size = 0;

					return THandle<void>();
				}
			}
		}

		return THandle<void>(handle);
	}

	ResourceArchiveZipFile::ResourceArchiveZipFile(ZipFileProxy *zipFileProxy, GpIOStream *stream, ResourceArchiveRef *resourceHandles)
		: m_zipFileProxy(zipFileProxy)
		, m_stream(stream)
		, m_resourceHandles(resourceHandles)
	{
	}

	ResourceArchiveZipFile::~ResourceArchiveZipFile()
	{
		MemoryManager *mm = MemoryManager::GetInstance();

		const size_t numFiles = m_zipFileProxy->NumFiles();
		for (size_t i = 0; i < numFiles; i++)
		{
			ResourceArchiveRef &ref = m_resourceHandles[numFiles - 1 - i];
			if (ref.m_handle)
				mm->ReleaseHandle(ref.m_handle);

			ref.~ResourceArchiveRef();
		}

		mm->Release(m_resourceHandles);

		m_zipFileProxy->Destroy();
		m_stream->Close();
	}

	// ========================================================================================

	ResourceArchiveDirectory *ResourceArchiveDirectory::Create(VirtualDirectory_t directory, const PLPasStr &subdirectory)
	{
		void *storage = PortabilityLayer::MemoryManager::GetInstance()->Alloc(sizeof(ResourceArchiveDirectory));
		if (!storage)
			return nullptr;

		return new (storage) ResourceArchiveDirectory(directory, subdirectory);
	}

	void ResourceArchiveDirectory::Destroy()
	{
		this->~ResourceArchiveDirectory();
		PortabilityLayer::MemoryManager::GetInstance()->Release(this);
	}

	THandle<void> ResourceArchiveDirectory::LoadResource(const ResTypeID &resTypeID, int id)
	{
		return GetResource(resTypeID, id, true);
	}

	bool ResourceArchiveDirectory::HasAnyResourcesOfType(const ResTypeID &resTypeID) const
	{
		int16_t scratch;
		return FindFirstResourceOfType(resTypeID, scratch);
	}

	bool ResourceArchiveDirectory::FindFirstResourceOfType(const ResTypeID &resTypeID, int16_t &outID) const
	{
		int32_t resID32 = resTypeID.ExportAsInt32();

		const ResTypeEntry *firstTypeEntry = *m_resTypes;
		const ResTypeEntry *lastTypeEntry = firstTypeEntry + m_numResourceTypes;

		const ResTypeEntry *entry = BinarySearch(firstTypeEntry, lastTypeEntry, resID32, ResourceArchiveDirectory::ResTypeSearchPredicate);

		if (entry == lastTypeEntry)
			return false;

		outID = (*m_resIDs)[entry->m_firstRes];
		return true;
	}

	bool ResourceArchiveDirectory::Init()
	{
		PortabilityLayer::HostFileSystem *fs = PortabilityLayer::HostFileSystem::GetInstance();

		const char *typePaths[1] = { this->m_subdirectory };

		PortabilityLayer::HostDirectoryCursor *typeDirCursor = fs->ScanDirectoryNested(m_directory, typePaths, 1);
		if (!typeDirCursor)
			return false;

		PortabilityLayer::MemoryManager *mm = PortabilityLayer::MemoryManager::GetInstance();

		m_resTypes = THandle<ResTypeEntry>(mm->AllocHandle(0));
		if (!m_resTypes)
			return false;

		m_resIDs = THandle<int16_t>(mm->AllocHandle(0));
		if (!m_resIDs)
			return false;

		size_t resTypeCapacity = 0;
		size_t resIDCapacity = 0;

		const char *typeScanFilename = nullptr;
		while (typeDirCursor->GetNext(typeScanFilename))
		{
			GpArcResourceTypeTag resourceTypeTag;
			if (!resourceTypeTag.Load(typeScanFilename))
				continue;

			ResTypeID resTypeID;
			if (!resourceTypeTag.Decode(resTypeID))
				continue;

			int32_t dirResType = resTypeID.ExportAsInt32();

			ResTypeEntry rte;
			rte.m_resTypeID = dirResType;
			rte.m_firstRes = m_numResources;
			rte.m_lastRes = m_numResources;

			const char *idScanFilenames[2] = { this->m_subdirectory, typeScanFilename };
			PortabilityLayer::HostDirectoryCursor *typeIDCursor = fs->ScanDirectoryNested(m_directory, idScanFilenames, 2);
			if (!typeIDCursor)
				continue;

			const char *idScanFilename = nullptr;
			while (typeIDCursor->GetNext(idScanFilename))
			{
				int resID = 0;
				bool isNegative = false;

				for (size_t chi = 0; idScanFilename[chi] != '.' && idScanFilename[chi] != '\0'; chi++)
				{
					char ch = idScanFilename[chi];
					if (ch == '-')
						isNegative = true;
					else if (ch >= '0' && ch <= '9')
					{
						resID *= 10;
						int digit = ch - '0';
						if (isNegative)
							resID -= digit;
						else
							resID += digit;
					}
					else
						break;
				}

				if (m_numResources == resIDCapacity)
				{
					const size_t oldCapacity = resIDCapacity;

					resIDCapacity *= 2;
					if (resIDCapacity == 0)
						resIDCapacity = 1;

					if (!mm->ResizeHandle(m_resIDs.MMBlock(), sizeof(int16_t) * resIDCapacity))
					{
						typeIDCursor->Destroy();
						typeDirCursor->Destroy();
						return false;
					}
				}

				(*m_resIDs)[m_numResources] = resID;
				m_numResources++;
			}

			typeIDCursor->Destroy();
			rte.m_lastRes = m_numResources;

			if (m_numResourceTypes == resTypeCapacity)
			{
				const size_t oldCapacity = resTypeCapacity;

				resTypeCapacity *= 2;
				if (resTypeCapacity == 0)
					resTypeCapacity = 1;

				if (!mm->ResizeHandle(m_resTypes.MMBlock(), sizeof(ResTypeEntry) * resTypeCapacity))
				{
					typeDirCursor->Destroy();
					return false;
				}
			}

			(*m_resTypes)[m_numResourceTypes] = rte;
			m_numResourceTypes++;
		}

		mm->ResizeHandle(m_resTypes.MMBlock(), sizeof(ResTypeEntry) * m_numResourceTypes);
		mm->ResizeHandle(m_resIDs.MMBlock(), sizeof(int16_t) * m_numResources);

		ResTypeEntry *resTypes = *m_resTypes;
		int16_t *resIDs = *m_resIDs;

		std::sort(resTypes, resTypes + m_numResourceTypes, ResourceArchiveDirectory::ResTypeEntrySortPredicate);

		for (size_t i = 0; i < m_numResourceTypes; i++)
		{
			int16_t *resIDStart = resIDs + resTypes[i].m_firstRes;
			int16_t *resIDEnd = resIDs + resTypes[i].m_lastRes;

			std::sort(resIDStart, resIDEnd);
		}

		m_resourceHandles = static_cast<ResourceArchiveRef*>(mm->Alloc(sizeof(ResourceArchiveRef) * m_numResources));
		if (!m_resourceHandles)
			return false;

		for (size_t i = 0; i < m_numResources; i++)
			new (m_resourceHandles + i) ResourceArchiveRef();

		return true;
	}

	bool ResourceArchiveDirectory::IndexResource(const ResTypeID &resTypeID, int id, size_t &outIndex) const
	{
		int32_t resID32 = resTypeID.ExportAsInt32();

		const ResTypeEntry *firstTypeEntry = *m_resTypes;
		const ResTypeEntry *lastTypeEntry = firstTypeEntry + m_numResourceTypes;

		const ResTypeEntry *entry = BinarySearch(firstTypeEntry, lastTypeEntry, resID32, ResourceArchiveDirectory::ResTypeSearchPredicate);

		if (entry == lastTypeEntry)
			return false;

		const int16_t *resIDs = *m_resIDs;
		const int16_t *firstRes = resIDs + entry->m_firstRes;
		const int16_t *lastRes = resIDs + entry->m_lastRes;

		const int16_t *idLoc = BinarySearch(firstRes, lastRes, static_cast<int16_t>(id), ResourceArchiveDirectory::ResIDSearchPredicate);
		if (idLoc == lastRes)
			return false;

		outIndex = static_cast<size_t>(idLoc - resIDs);

		return true;
	}

	THandle<void> ResourceArchiveDirectory::GetResource(const ResTypeID &resTypeID, int id, bool load)
	{

		int validationRule = 0;
		size_t index = 0;
		if (!IndexResource(resTypeID, id, index))
			return THandle<void>();

		ResourceArchiveRef *ref = m_resourceHandles + index;

		MMHandleBlock *handle = nullptr;
		if (ref->m_handle != nullptr)
			handle = ref->m_handle;
		else
		{
			handle = MemoryManager::GetInstance()->AllocHandle(0);
			if (!handle)
				return THandle<void>();

			handle->m_rmSelfRef = ref;
			ref->m_handle = handle;
			ref->m_resID = static_cast<int16_t>(id);
			ref->m_size = 0;
		}

		if (handle->m_contents == nullptr && load)
		{
			int validationRule = 0;
			const char *extension = GetFileExtensionForResType(resTypeID, validationRule);

			GpArcResourceTypeTag resTypeTag = GpArcResourceTypeTag::Encode(resTypeID);
			char fileName[32];

			snprintf(fileName, sizeof(fileName) - 1, "%i%s", id, extension);

			const char *paths[3] = { m_subdirectory, resTypeTag.m_id, fileName };

			GpIOStream *ioStream = PortabilityLayer::HostFileSystem::GetInstance()->OpenFileNested(m_directory, paths, 3, false, GpFileCreationDispositions::kOpenExisting);
			if (!ioStream)
				return THandle<void>();

			size_t size = ioStream->Size();

			void *contents = MemoryManager::GetInstance()->Alloc(size);
			handle->m_contents = contents;
			handle->m_size = size;
			ref->m_size = size;

			bool readOK = (ioStream->Read(contents, size));
			ioStream->Close();

			if (!readOK || (validationRule != ResourceValidationRules::kNone && !ValidateResource(contents, ref->m_size, static_cast<ResourceValidationRule_t>(validationRule))))
			{
				MemoryManager::GetInstance()->Release(contents);
				handle->m_contents = nullptr;
				handle->m_size = 0;
				ref->m_size = 0;

				return THandle<void>();
			}
		}

		return THandle<void>(handle);
	}

	int ResourceArchiveDirectory::ResTypeSearchPredicate(int32_t resTypeID, const ResTypeEntry &entry)
	{
		if (resTypeID < entry.m_resTypeID)
			return -1;
		if (resTypeID > entry.m_resTypeID)
			return 1;
		return 0;
	}

	int ResourceArchiveDirectory::ResIDSearchPredicate(int16_t resTypeID, int16_t entry)
	{
		if (resTypeID < entry)
			return -1;
		if (resTypeID > entry)
			return 1;
		return 0;
	}

	bool ResourceArchiveDirectory::ResTypeEntrySortPredicate(const ResTypeEntry &a, const ResTypeEntry &b)
	{
		return a.m_resTypeID < b.m_resTypeID;
	}

	ResourceArchiveDirectory::ResourceArchiveDirectory(VirtualDirectory_t directory, const PLPasStr &subdirectory)
		: m_directory(directory)
		, m_numResourceTypes(0)
		, m_resourceHandles(nullptr)
		, m_numResources(0)
	{
		memcpy(m_subdirectory, subdirectory.UChars(), subdirectory.Length());
		m_subdirectory[subdirectory.Length()] = '\0';
	}

	ResourceArchiveDirectory::~ResourceArchiveDirectory()
	{
		MemoryManager *mm = MemoryManager::GetInstance();

		const size_t numHandles = m_numResources;

		if (m_resourceHandles)
		{
			for (size_t i = 0; i < numHandles; i++)
			{
				ResourceArchiveRef &ref = m_resourceHandles[numHandles - 1 - i];
				if (ref.m_handle)
					mm->ReleaseHandle(ref.m_handle);

				ref.~ResourceArchiveRef();
			}
		}

		mm->Release(m_resourceHandles);

		m_resIDs.Dispose();
		m_resTypes.Dispose();
	}
}
