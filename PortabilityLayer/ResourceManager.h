#pragma once

#include "VirtualDirectory.h"
#include "PascalStr.h"
#include "PLErrorCodes.h"
#include "PLHandle.h"

class PLPasStr;

class GpIOStream;

namespace PortabilityLayer
{
	struct MMHandleBlock;
	struct ResourceCompiledRef;
	class ResourceFile;
	class ResTypeID;
	class ZipFileProxy;

	struct ResourceArchiveRef
	{
		ResourceArchiveRef();

		MMHandleBlock *m_handle;
		size_t m_size;
		int16_t m_resID;
	};

	struct IResourceArchive
	{
		virtual void Destroy() = 0;

		virtual THandle<void> LoadResource(const ResTypeID &resTypeID, int id) = 0;

		virtual bool HasAnyResourcesOfType(const ResTypeID &resTypeID) const = 0;
		virtual bool FindFirstResourceOfType(const ResTypeID &resTypeID, int16_t &outID) const = 0;
	};

	class ResourceArchiveBase : public IResourceArchive
	{
	public:
		static const char *GetFileExtensionForResType(const ResTypeID &resTypeID, int &outValidationRule);
	};

	class ResourceArchiveDirectory final : public ResourceArchiveBase
	{
	public:
		static ResourceArchiveDirectory *Create(VirtualDirectory_t directory, const PLPasStr &subdirectory);
		void Destroy() override;

		THandle<void> LoadResource(const ResTypeID &resTypeID, int id) override;

		bool HasAnyResourcesOfType(const ResTypeID &resTypeID) const override;
		bool FindFirstResourceOfType(const ResTypeID &resTypeID, int16_t &outID) const override;

		bool Init();

	private:
		ResourceArchiveDirectory(VirtualDirectory_t directory, const PLPasStr &subdirectory);
		~ResourceArchiveDirectory();

		struct ResTypeEntry
		{
			int32_t m_resTypeID;
			size_t m_firstRes;
			size_t m_lastRes;
		};

		bool IndexResource(const ResTypeID &resTypeID, int id, size_t &outIndex) const;
		THandle<void> GetResource(const ResTypeID &resTypeID, int id, bool load);

		static int ResTypeSearchPredicate(int32_t resTypeID, const ResTypeEntry &entry);
		static int ResIDSearchPredicate(int16_t resTypeID, int16_t entry);
		static bool ResTypeEntrySortPredicate(const ResTypeEntry &a, const ResTypeEntry &b);

		VirtualDirectory_t m_directory;
		char m_subdirectory[256];

		THandle<ResTypeEntry> m_resTypes;
		size_t m_numResourceTypes;

		THandle<int16_t> m_resIDs;
		ResourceArchiveRef *m_resourceHandles;
		size_t m_numResources;
	};

	class ResourceArchiveZipFile final : public ResourceArchiveBase
	{
	public:
		static ResourceArchiveZipFile *Create(ZipFileProxy *zipFileProxy, GpIOStream *stream);
		void Destroy() override;

		THandle<void> LoadResource(const ResTypeID &resTypeID, int id) override;

		bool HasAnyResourcesOfType(const ResTypeID &resTypeID) const override;
		bool FindFirstResourceOfType(const ResTypeID &resTypeID, int16_t &outID) const override;

	private:
		ResourceArchiveZipFile(ZipFileProxy *zipFileProxy, GpIOStream *stream, ResourceArchiveRef *resourceHandles);
		~ResourceArchiveZipFile();

		bool IndexResource(const ResTypeID &resTypeID, int id, size_t &outIndex, int &outValidationRule) const;

		THandle<void> GetResource(const ResTypeID &resTypeID, int id, bool load);

		ZipFileProxy *m_zipFileProxy;
		GpIOStream *m_stream;
		ResourceArchiveRef *m_resourceHandles;
	};

	class ResourceManager
	{
	public:
		virtual void Init() = 0;
		virtual void Shutdown() = 0;

		virtual THandle<void> GetAppResource(const ResTypeID &resTypeID, int16_t resID) const = 0;
		virtual IResourceArchive *GetAppResourceArchive() const = 0;

		virtual IResourceArchive *LoadResFile(VirtualDirectory_t virtualDir, const PLPasStr &filename) const = 0;
		virtual PLError_t CreateBlankResFile(VirtualDirectory_t virtualDir, const PLPasStr &filename) = 0;

		virtual void DissociateHandle(MMHandleBlock *hdl) const = 0;
		virtual const ResourceArchiveRef *ResourceForHandle(MMHandleBlock *hdl) const = 0;

		static ResourceManager *GetInstance();
	};
}
