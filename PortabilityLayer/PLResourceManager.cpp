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

		short OpenResFork(EVirtualDirectory virtualDir, const PLPasStr &filename) override;
		MMHandleBlock *GetResource(const ResTypeID &resType, int id) override;

		short GetCurrentResFile() const override;
		void SetCurrentResFile(short ref) override;

		static ResourceManagerImpl *GetInstance();

	private:
		std::vector<ResourceFile*> m_resFiles;
		short m_currentResFile;
		bool m_load;

		static ResourceManagerImpl ms_instance;
	};

	ResourceManagerImpl::ResourceManagerImpl()
		: m_currentResFile(0)
		, m_load(true)
	{
	}

	void ResourceManagerImpl::Init()
	{
		m_currentResFile = OpenResFork(EVirtualDirectory_ApplicationData, PSTR("ApplicationResources"));
	}

	void ResourceManagerImpl::Shutdown()
	{
		for (std::vector<ResourceFile*>::iterator it = m_resFiles.begin(), itEnd = m_resFiles.end(); it != itEnd; ++it)
			delete (*it);

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

	short ResourceManagerImpl::OpenResFork(EVirtualDirectory virtualDir, const PLPasStr &filename)
	{
		const size_t numSlots = m_resFiles.size();
		size_t resFileIndex = numSlots;

		for (size_t i = 0; i < numSlots; i++)
		{
			if (m_resFiles[i] == nullptr)
			{
				resFileIndex = i;
				break;
			}
		}

		if (resFileIndex == 0x7fff)
			return 0;

		IOStream *fStream = nullptr;
		if (FileManager::GetInstance()->RawOpenFileRF(virtualDir, filename, EFilePermission_Read, true, &fStream) != noErr)
			return 0;

		ResourceFile *resFile = new ResourceFile();
		bool loaded = resFile->Load(fStream);
		fStream->Close();

		if (!loaded)
		{
			delete resFile;
			return 0;
		}

		if (resFileIndex == numSlots)
			m_resFiles.push_back(resFile);
		else
			m_resFiles[resFileIndex] = resFile;

		return static_cast<short>(resFileIndex + 1);
	}

	MMHandleBlock *ResourceManagerImpl::GetResource(const ResTypeID &resType, int id)
	{
		if (!m_currentResFile)
			return nullptr;

		ResourceFile *resFile = m_resFiles[m_currentResFile - 1];
		if (!resFile)
			return nullptr;

		return resFile->GetResource(resType, id, m_load);
	}


	ResourceManagerImpl ResourceManagerImpl::ms_instance;

	ResourceManager *ResourceManager::GetInstance() { return ResourceManagerImpl::GetInstance(); }
}
