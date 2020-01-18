#include "ResourceManager.h"
#include "VirtualDirectory.h"
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
#include "ZipFileProxy.h"

#include <vector>

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
		
		void SetResLoad(bool load) override;

		ResourceArchive *LoadResFile(VirtualDirectory_t virtualDir, const PLPasStr &filename) const override;
		short OpenResFork(VirtualDirectory_t virtualDir, const PLPasStr &filename) override;
		void CloseResFile(short ref) override;
		PLError_t CreateBlankResFile(VirtualDirectory_t virtualDir, const PLPasStr &filename) override;

		THandle<void> GetResource(const ResTypeID &resType, int id) override;

		short GetCurrentResFile() const override;
		void SetCurrentResFile(short ref) override;

		void DissociateHandle(MMHandleBlock *hdl) const override;
		const ResourceArchiveRef *ResourceForHandle(MMHandleBlock *hdl) const override;

		static ResourceManagerImpl *GetInstance();

	private:
		struct ResFileSlot
		{
			short m_prevFile;
			short m_nextFile;
			ResourceArchive *m_resourceArchive;
		};

		void UnloadAndDestroyResourceFile(ResourceArchive *rf);

		std::vector<ResFileSlot> m_resFiles;
		short m_firstResFile;
		short m_lastResFile;
		short m_currentResFile;
		bool m_load;

		static ResourceManagerImpl ms_instance;
	};

	ResourceManagerImpl::ResourceManagerImpl()
		: m_currentResFile(-1)
		, m_firstResFile(-1)
		, m_lastResFile(-1)
		, m_load(true)
	{
	}

	void ResourceManagerImpl::Init()
	{
		m_currentResFile = OpenResFork(VirtualDirectories::kApplicationData, PSTR("ApplicationResources"));
	}

	void ResourceManagerImpl::Shutdown()
	{
		for (std::vector<ResFileSlot>::iterator it = m_resFiles.begin(), itEnd = m_resFiles.end(); it != itEnd; ++it)
		{
			if (it->m_resourceArchive)
				UnloadAndDestroyResourceFile(it->m_resourceArchive);
		}

		m_resFiles.clear();
	}
	
	short ResourceManagerImpl::GetCurrentResFile() const
	{
		return m_currentResFile;
	}

	void ResourceManagerImpl::SetCurrentResFile(short ref)
	{
		m_currentResFile = ref;
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

	void ResourceManagerImpl::SetResLoad(bool load)
	{
		m_load = load;
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

	short ResourceManagerImpl::OpenResFork(VirtualDirectory_t virtualDir, const PLPasStr &filename)
	{
		const size_t numSlots = m_resFiles.size();
		size_t resFileIndex = numSlots;

		for (size_t i = 0; i < numSlots; i++)
		{
			if (m_resFiles[i].m_resourceArchive == nullptr)
			{
				resFileIndex = i;
				break;
			}
		}

		if (resFileIndex == 0x7fff)
			return -1;


		ResourceArchive *resFile = LoadResFile(virtualDir, filename);

		if (!resFile)
			return -1;

		ResFileSlot slot;
		slot.m_resourceArchive = resFile;
		slot.m_prevFile = m_lastResFile;
		slot.m_nextFile = -1;

		if (resFileIndex == numSlots)
			m_resFiles.push_back(slot);
		else
			m_resFiles[resFileIndex] = slot;

		const short rfid = static_cast<short>(resFileIndex);

		if (m_firstResFile < 0)
			m_firstResFile = rfid;

		if (m_lastResFile >= 0)
			m_resFiles[m_lastResFile].m_nextFile = rfid;

		m_lastResFile = rfid;
		m_currentResFile = rfid;	// Resource Manager is supposed to reset the search stack on new file open

		return rfid;
	}

	void ResourceManagerImpl::CloseResFile(short ref)
	{
		ResFileSlot &slot = m_resFiles[ref];

		assert(slot.m_resourceArchive != nullptr);

		slot.m_resourceArchive->Destroy();
		slot.m_resourceArchive = nullptr;

		if (m_lastResFile == ref)
			m_lastResFile = slot.m_prevFile;

		if (slot.m_prevFile >= 0)
		{
			ResFileSlot &prevFileSlot = m_resFiles[slot.m_prevFile];
			prevFileSlot.m_nextFile = slot.m_nextFile;
		}

		if (slot.m_nextFile >= 0)
		{
			ResFileSlot &nextFileSlot = m_resFiles[slot.m_nextFile];
			nextFileSlot.m_prevFile = slot.m_prevFile;
		}

		slot.m_nextFile = slot.m_prevFile = -1;

		m_currentResFile = m_lastResFile;
	}

	PLError_t ResourceManagerImpl::CreateBlankResFile(VirtualDirectory_t virtualDir, const PLPasStr &filename)
	{
		const uint8_t blankResFileData[] = {
			0, 0, 0, 16, 0, 0,  0, 16, 0, 0,  0, 0,  0,   0,  0, 30,
			0, 0, 0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0,   0,  0, 0,
			0, 0, 0, 0,  0, 0,  0, 0,  0, 28, 0, 30, 255, 255 };

		PortabilityLayer::IOStream *stream = nullptr;
		PLError_t error = FileManager::GetInstance()->RawOpenFileResources(virtualDir, filename, EFilePermission_Write, true, GpFileCreationDispositions::kCreateOrOverwrite, stream);
		if (error)
			return error;

		if (stream->Write(blankResFileData, sizeof(blankResFileData)) != sizeof(blankResFileData))
		{
			stream->Close();
			return PLErrors::kIOError;
		}

		stream->Close();

		return PLErrors::kNone;
	}

	THandle<void> ResourceManagerImpl::GetResource(const ResTypeID &resType, int id)
	{
		short searchIndex = m_currentResFile;
		while (searchIndex >= 0)
		{
			const ResFileSlot& slot = m_resFiles[searchIndex];
			assert(slot.m_resourceArchive);

			THandle<void> resHdl = slot.m_resourceArchive->GetResource(resType, id, m_load);
			if (resHdl != nullptr)
				return resHdl;

			searchIndex = slot.m_prevFile;
		}

		return nullptr;
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

		ResourceArchiveRef *refs = static_cast<ResourceArchiveRef*>(mm->Alloc(sizeof(ResourceArchiveRef) * numFiles));
		if (!refs)
			return nullptr;

		for (size_t i = 0; i < numFiles; i++)
			new (refs + i) ResourceArchiveRef();

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

	THandle<void> ResourceArchive::GetResource(const ResTypeID &resTypeID, int id, bool load)
	{
		const char *extension = ".bin";
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

				if (!m_zipFileProxy->LoadFile(index, contents))
				{
					MemoryManager::GetInstance()->Release(contents);
					handle->m_contents = nullptr;
					handle->m_size = 0;
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
