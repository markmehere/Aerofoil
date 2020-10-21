#pragma once

#include "GpFileCreationDisposition.h"
#include "VirtualDirectory.h"

#include <stdint.h>

class GpIOStream;
struct IGpThreadRelay;

namespace PortabilityLayer
{
	class HostDirectoryCursor;

	class HostFileSystem
	{
	public:
		virtual bool FileExists(VirtualDirectory_t virtualDirectory, const char *path) = 0;
		virtual bool FileLocked(VirtualDirectory_t virtualDirectory, const char *path, bool *exists) = 0;
		virtual GpIOStream *OpenFileNested(PortabilityLayer::VirtualDirectory_t virtualDirectory, char const* const* subPaths, size_t numSubPaths, bool writeAccess, GpFileCreationDisposition_t createDisposition) = 0;
		virtual bool DeleteFile(VirtualDirectory_t virtualDirectory, const char *path, bool &existed) = 0;
		virtual HostDirectoryCursor *ScanDirectoryNested(VirtualDirectory_t virtualDirectory, char const* const* paths, size_t numPaths) = 0;
		HostDirectoryCursor *ScanDirectory(VirtualDirectory_t virtualDirectory);

		virtual bool ValidateFilePath(const char *path, size_t pathLen) const = 0;
		virtual bool ValidateFilePathUnicodeChar(uint32_t ch) const = 0;

		virtual bool IsVirtualDirectoryLooseResources(VirtualDirectory_t virtualDir) const = 0;

		static HostFileSystem *GetInstance();
		static void SetInstance(HostFileSystem *instance);

		GpIOStream *OpenFile(VirtualDirectory_t virtualDirectory, const char *path, bool writeAccess, GpFileCreationDisposition_t createDisposition);

		virtual void SetMainThreadRelay(IGpThreadRelay *relay) = 0;

	private:
		static HostFileSystem *ms_instance;
	};
}

inline GpIOStream *PortabilityLayer::HostFileSystem::OpenFile(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *path, bool writeAccess, GpFileCreationDisposition_t createDisposition)
{
	return this->OpenFileNested(virtualDirectory, &path, 1, writeAccess, createDisposition);
}

inline PortabilityLayer::HostDirectoryCursor *PortabilityLayer::HostFileSystem::ScanDirectory(VirtualDirectory_t virtualDirectory)
{
	return this->ScanDirectoryNested(virtualDirectory, nullptr, 0);
}
