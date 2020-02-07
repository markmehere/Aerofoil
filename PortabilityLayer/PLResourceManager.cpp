#include "ResourceManager.h"

#include "BMPFormat.h"
#include "FileManager.h"
#include "GPArchive.h"
#include "HostFileSystem.h"
#include "HostMemoryBuffer.h"
#include "IOStream.h"
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

namespace ResourceValidationRules
{
	enum ResourceValidationRule
	{
		kNone,
		kWAV,
		kBMP,
	};
}

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
	class ResourceArchive;

	class ResourceManagerImpl final : public ResourceManager
	{
	public:
		ResourceManagerImpl();

		void Init() override;
		void Shutdown() override;

		THandle<void> GetAppResource(const ResTypeID &resTypeID, int16_t resID) const override;
		ResourceArchive *GetAppResourceArchive() const override;

		ResourceArchive *LoadResFile(VirtualDirectory_t virtualDir, const PLPasStr &filename) const override;
		PLError_t CreateBlankResFile(VirtualDirectory_t virtualDir, const PLPasStr &filename) override;

		void DissociateHandle(MMHandleBlock *hdl) const override;
		const ResourceArchiveRef *ResourceForHandle(MMHandleBlock *hdl) const override;

		static ResourceManagerImpl *GetInstance();

	private:
		void UnloadAndDestroyResourceFile(ResourceArchive *rf);

		ResourceArchive *m_appResArchive;

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

	void ResourceManagerImpl::UnloadAndDestroyResourceFile(ResourceArchive *rf)
	{
		rf->Destroy();
	}

	ResourceManagerImpl *ResourceManagerImpl::GetInstance()
	{
		return &ms_instance;
	}

	ResourceArchive *ResourceManagerImpl::LoadResFile(VirtualDirectory_t virtualDir, const PLPasStr &filename) const
	{
		IOStream *fStream = nullptr;
		if (FileManager::GetInstance()->RawOpenFileResources(virtualDir, filename, EFilePermission_Read, true, GpFileCreationDispositions::kOpenExisting, fStream) != PLErrors::kNone)
			return nullptr;

		ZipFileProxy *proxy = ZipFileProxy::Create(fStream);
		if (!proxy)
		{
			fStream->Close();
			return nullptr;
		}

		ResourceArchive *archive = ResourceArchive::Create(proxy, fStream);
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
		PortabilityLayer::IOStream *stream = nullptr;
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

	ResourceArchive *ResourceManagerImpl::GetAppResourceArchive() const
	{
		return m_appResArchive;
	}

	ResourceManagerImpl ResourceManagerImpl::ms_instance;

	ResourceManager *ResourceManager::GetInstance() { return ResourceManagerImpl::GetInstance(); }

	// ===========================================================================================

	ResourceArchiveRef::ResourceArchiveRef()
		: m_handle(nullptr)
		, m_size(0)
		, m_resID(0)
	{
	}

	ResourceArchive *ResourceArchive::Create(ZipFileProxy *zipFileProxy, IOStream *stream)
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

		void *storage = mm->Alloc(sizeof(ResourceArchive));
		if (!storage)
		{
			mm->Release(refs);
			return nullptr;
		}

		return new (storage) ResourceArchive(zipFileProxy, stream, refs);
	}

	void ResourceArchive::Destroy()
	{
		this->~ResourceArchive();
		PortabilityLayer::MemoryManager::GetInstance()->Release(this);
	}

	THandle<void> ResourceArchive::LoadResource(const ResTypeID &resTypeID, int id)
	{
		return GetResource(resTypeID, id, true);
	}

	bool ResourceArchive::GetResourceSize(const ResTypeID &resTypeID, int id, size_t &outSize)
	{
		THandle<void> hdl = GetResource(resTypeID, id, false);
		if (const PortabilityLayer::MMHandleBlock *hdlBlock = hdl.MMBlock())
		{
			outSize = hdlBlock->m_rmSelfRef->m_size;
			return true;
		}

		return false;
	}

	THandle<void> ResourceArchive::GetResource(const ResTypeID &resTypeID, int id, bool load)
	{
		const char *extension = ".bin";
		ResourceValidationRule_t validationRule = ResourceValidationRules::kNone;

		if (resTypeID == ResTypeID('snd '))
		{
			extension = ".wav";
			validationRule = ResourceValidationRules::kWAV;
		}
		else if (resTypeID == ResTypeID('Date') || resTypeID == ResTypeID('PICT'))
		{
			extension = ".bmp";
			validationRule = ResourceValidationRules::kBMP;
		}
		else if (resTypeID == ResTypeID('STR#'))
			extension = ".txt";
		else if (resTypeID == ResTypeID('DITL'))
			extension = ".json";

		char resourceFile[64];

		GpArcResourceTypeTag resTag = GpArcResourceTypeTag::Encode(resTypeID);

		snprintf(resourceFile, sizeof(resourceFile) - 1, "%s/%i%s", resTag.m_id, id, extension);

		size_t index = 0;
		if (!m_zipFileProxy->IndexFile(resourceFile, index))
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

				if (!m_zipFileProxy->LoadFile(index, contents) || (validationRule != ResourceValidationRules::kNone && !ValidateResource(contents, ref->m_size, validationRule)))
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

	ResourceArchive::ResourceArchive(ZipFileProxy *zipFileProxy, IOStream *stream, ResourceArchiveRef *resourceHandles)
		: m_zipFileProxy(zipFileProxy)
		, m_stream(stream)
		, m_resourceHandles(resourceHandles)
	{
	}

	ResourceArchive::~ResourceArchive()
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
}
