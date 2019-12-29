#include "ResourceManager.h"
#include "VirtualDirectory.h"
#include "FileManager.h"
#include "HostFileSystem.h"
#include "HostMemoryBuffer.h"
#include "IOStream.h"
#include "MacBinary2.h"
#include "MacFileMem.h"
#include "MemReaderStream.h"
#include "ResourceFile.h"
#include "PLPasStr.h"
#include "PLErrorCodes.h"

#include <vector>

namespace PortabilityLayer
{
	class ResourceManagerImpl final : public ResourceManager
	{
	public:
		ResourceManagerImpl();

		void Init() override;
		void Shutdown() override;
		
		void SetResLoad(bool load) override;

		short OpenResFork(VirtualDirectory_t virtualDir, const PLPasStr &filename) override;
		MMHandleBlock *GetResource(const ResTypeID &resType, int id) override;

		short GetCurrentResFile() const override;
		void SetCurrentResFile(short ref) override;

		static ResourceManagerImpl *GetInstance();

	private:
		struct ResFileSlot
		{
			short m_prevFile;
			short m_nextFile;
			ResourceFile* m_resourceFile;
		};

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
			delete it->m_resourceFile;

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
		if (FileManager::GetInstance()->RawOpenFileRF(virtualDir, filename, EFilePermission_Read, true, fStream) != PLErrors::kNone)
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

	MMHandleBlock *ResourceManagerImpl::GetResource(const ResTypeID &resType, int id)
	{
		short searchIndex = m_currentResFile;
		while (searchIndex >= 0)
		{
			const ResFileSlot& slot = m_resFiles[searchIndex];
			assert(slot.m_resourceFile);

			if (MMHandleBlock *block = slot.m_resourceFile->GetResource(resType, id, m_load))
				return block;

			searchIndex = slot.m_prevFile;
		}

		return nullptr;
	}


	ResourceManagerImpl ResourceManagerImpl::ms_instance;

	ResourceManager *ResourceManager::GetInstance() { return ResourceManagerImpl::GetInstance(); }
}
