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

		THandle<void> GetResource(const ResTypeID &resTypeID, int id, bool load);

	private:
		ResourceArchive(ZipFileProxy *zipFileProxy, IOStream *stream, ResourceArchiveRef *resourceHandles);
		~ResourceArchive();

		ZipFileProxy *m_zipFileProxy;
		IOStream *m_stream;
		ResourceArchiveRef *m_resourceHandles;
	};

	class ResourceManager
	{
	public:
		virtual void Init() = 0;
		virtual void Shutdown() = 0;

		virtual void SetResLoad(bool load) = 0;

		virtual ResourceArchive *LoadResFile(VirtualDirectory_t virtualDir, const PLPasStr &filename) const = 0;
		virtual short OpenResFork(VirtualDirectory_t virtualDir, const PLPasStr &filename) = 0;
		virtual void CloseResFile(short ref) = 0;
		virtual PLError_t CreateBlankResFile(VirtualDirectory_t virtualDir, const PLPasStr &filename) = 0;

		virtual THandle<void> GetResource(const ResTypeID &resType, int id) = 0;

		virtual short GetCurrentResFile() const = 0;
		virtual void SetCurrentResFile(short ref) = 0;

		virtual void DissociateHandle(MMHandleBlock *hdl) const = 0;
		virtual const ResourceArchiveRef *ResourceForHandle(MMHandleBlock *hdl) const = 0;

		static ResourceManager *GetInstance();
	};
}
