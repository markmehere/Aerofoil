#include "ResourceManager.h"
#include "VirtualDirectory.h"
#include "FileManager.h"
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

#include <vector>

namespace PortabilityLayer
{
	struct MMHandleBlock;

	class ResourceManagerImpl final : public ResourceManager
	{
	public:
		ResourceManagerImpl();

		void Init() override;
		void Shutdown() override;
		
		void SetResLoad(bool load) override;

		short OpenResFork(VirtualDirectory_t virtualDir, const PLPasStr &filename) override;
		void CloseResFile(short ref) override;
		PLError_t CreateBlankResFile(VirtualDirectory_t virtualDir, const PLPasStr &filename) override;

		THandle<void> GetResource(const ResTypeID &resType, int id) override;

		short GetCurrentResFile() const override;
		void SetCurrentResFile(short ref) override;

		void DissociateHandle(MMHandleBlock *hdl) const override;
		const ResourceCompiledRef *ResourceForHandle(MMHandleBlock *hdl) const override;

		static ResourceManagerImpl *GetInstance();

	private:
		struct ResFileSlot
		{
			short m_prevFile;
			short m_nextFile;
			ResourceFile *m_resourceFile;
		};

		void UnloadAndDestroyResourceFile(ResourceFile *rf);

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
			if (it->m_resourceFile)
				UnloadAndDestroyResourceFile(it->m_resourceFile);
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

	const ResourceCompiledRef *ResourceManagerImpl::ResourceForHandle(MMHandleBlock *hdl) const
	{
		return hdl->m_rmSelfRef;
	}

	void ResourceManagerImpl::UnloadAndDestroyResourceFile(ResourceFile *rf)
	{
		PortabilityLayer::MemoryManager *mm = PortabilityLayer::MemoryManager::GetInstance();

		ResourceCompiledTypeList *rtls = nullptr;
		size_t numRTLs = 0;
		rf->GetAllResourceTypeLists(rtls, numRTLs);

		for (size_t i = 0; i < numRTLs; i++)
		{
			const ResourceCompiledTypeList &rtl = rtls[i];

			const size_t numRefs = rtl.m_numRefs;
			for (size_t r = 0; r < numRefs; r++)
			{
				const ResourceCompiledRef &ref = rtl.m_firstRef[r];
				if (MMHandleBlock *hdl = ref.m_handle)
					mm->ReleaseHandle(hdl);
			}
		}

		delete rf;
	}

	ResourceManagerImpl *ResourceManagerImpl::GetInstance()
	{
		return &ms_instance;
	}

	void ResourceManagerImpl::SetResLoad(bool load)
	{
		m_load = load;
	}

	short ResourceManagerImpl::OpenResFork(VirtualDirectory_t virtualDir, const PLPasStr &filename)
	{
		const size_t numSlots = m_resFiles.size();
		size_t resFileIndex = numSlots;

		for (size_t i = 0; i < numSlots; i++)
		{
			if (m_resFiles[i].m_resourceFile == nullptr)
			{
				resFileIndex = i;
				break;
			}
		}

		if (resFileIndex == 0x7fff)
			return -1;

		IOStream *fStream = nullptr;
		if (FileManager::GetInstance()->RawOpenFileResources(virtualDir, filename, EFilePermission_Read, true, GpFileCreationDispositions::kOpenExisting, fStream) != PLErrors::kNone)
			return -1;

		ResourceFile *resFile = new ResourceFile();
		bool loaded = resFile->Load(fStream);
		fStream->Close();

		if (!loaded)
		{
			delete resFile;
			return -1;
		}

		ResFileSlot slot;
		slot.m_resourceFile = resFile;
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
		delete slot.m_resourceFile;
		slot.m_resourceFile = nullptr;

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
			assert(slot.m_resourceFile);

			if (MMHandleBlock *block = slot.m_resourceFile->GetResource(resType, id, m_load))
				return THandle<void>(block);

			searchIndex = slot.m_prevFile;
		}

		return nullptr;
	}


	ResourceManagerImpl ResourceManagerImpl::ms_instance;

	ResourceManager *ResourceManager::GetInstance() { return ResourceManagerImpl::GetInstance(); }
}
