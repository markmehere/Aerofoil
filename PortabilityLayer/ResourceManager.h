#pragma once

#include "VirtualDirectory.h"
#include "PLErrorCodes.h"
#include "PLHandle.h"

class PLPasStr;

namespace PortabilityLayer
{
	class IOStream;
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

	class ResourceArchive final
	{
	public:
		static ResourceArchive *Create(ZipFileProxy *zipFileProxy, IOStream *stream);
		void Destroy();

		THandle<void> LoadResource(const ResTypeID &resTypeID, int id);
		bool GetResourceSize(const ResTypeID &resTypeID, int id, size_t &outSize) const;

		bool HasAnyResourcesOfType(const ResTypeID &resTypeID) const;
		bool FindFirstResourceOfType(const ResTypeID &resTypeID, int16_t &outID) const;

	private:
		ResourceArchive(ZipFileProxy *zipFileProxy, IOStream *stream, ResourceArchiveRef *resourceHandles);
		~ResourceArchive();

		bool IndexResource(const ResTypeID &resTypeID, int id, size_t &outIndex, int &outValidationRule) const;

		THandle<void> GetResource(const ResTypeID &resTypeID, int id, bool load);

		ZipFileProxy *m_zipFileProxy;
		IOStream *m_stream;
		ResourceArchiveRef *m_resourceHandles;
	};

	class ResourceManager
	{
	public:
		virtual void Init() = 0;
		virtual void Shutdown() = 0;

		virtual THandle<void> GetAppResource(const ResTypeID &resTypeID, int16_t resID) const = 0;
		virtual ResourceArchive *GetAppResourceArchive() const = 0;

		virtual ResourceArchive *LoadResFile(VirtualDirectory_t virtualDir, const PLPasStr &filename) const = 0;
		virtual PLError_t CreateBlankResFile(VirtualDirectory_t virtualDir, const PLPasStr &filename) = 0;

		virtual void DissociateHandle(MMHandleBlock *hdl) const = 0;
		virtual const ResourceArchiveRef *ResourceForHandle(MMHandleBlock *hdl) const = 0;

		static ResourceManager *GetInstance();
	};
}
