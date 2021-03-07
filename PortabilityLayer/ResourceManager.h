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
	class CompositeFile;

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

	class ResourceArchiveZipFile final : public ResourceArchiveBase
	{
	public:
		static ResourceArchiveZipFile *Create(ZipFileProxy *zipFileProxy, bool proxyIsShared, GpIOStream *stream);
		void Destroy() override;

		THandle<void> LoadResource(const ResTypeID &resTypeID, int id) override;

		bool HasAnyResourcesOfType(const ResTypeID &resTypeID) const override;
		bool FindFirstResourceOfType(const ResTypeID &resTypeID, int16_t &outID) const override;

	private:
		ResourceArchiveZipFile(ZipFileProxy *zipFileProxy, bool proxyIsShared, GpIOStream *stream, ResourceArchiveRef *resourceHandles);
		~ResourceArchiveZipFile();

		bool IndexResource(const ResTypeID &resTypeID, int id, size_t &outIndex, int &outValidationRule) const;

		THandle<void> GetResource(const ResTypeID &resTypeID, int id, bool load);

		ZipFileProxy *m_zipFileProxy;
		GpIOStream *m_stream;	// This may be null, i.e. a composite file may own it instead
		ResourceArchiveRef *m_resourceHandles;
		bool m_proxyIsShared;
	};

	class ResourceManager
	{
	public:
		virtual void Init() = 0;
		virtual void Shutdown() = 0;

		virtual THandle<void> GetAppResource(const ResTypeID &resTypeID, int16_t resID) const = 0;
		virtual IResourceArchive *GetAppResourceArchive() const = 0;

		virtual IResourceArchive *LoadResFile(CompositeFile *file) const = 0;
		virtual PLError_t CreateBlankResFile(VirtualDirectory_t virtualDir, const PLPasStr &filename) = 0;

		virtual void DissociateHandle(MMHandleBlock *hdl) const = 0;
		virtual const ResourceArchiveRef *ResourceForHandle(MMHandleBlock *hdl) const = 0;

		static ResourceManager *GetInstance();
	};
}
