#pragma once

#include "GpFileCreationDisposition.h"
#include "VirtualDirectory.h"

#include <stdint.h>

namespace PortabilityLayer
{
	class IOStream;
	class HostDirectoryCursor;

	class HostFileSystem
	{
	public:
		virtual bool FileExists(VirtualDirectory_t virtualDirectory, const char *path) = 0;
		virtual bool FileLocked(VirtualDirectory_t virtualDirectory, const char *path, bool *exists) = 0;
		virtual IOStream *OpenFile(VirtualDirectory_t virtualDirectory, const char *path, bool writeAccess, GpFileCreationDisposition_t createDisposition) = 0;
		virtual bool DeleteFile(VirtualDirectory_t virtualDirectory, const char *path, bool &existed) = 0;
		virtual HostDirectoryCursor *ScanDirectory(VirtualDirectory_t virtualDirectory) = 0;

		virtual bool PromptSaveFile(VirtualDirectory_t virtualDirectory, char *path, size_t &outPathLength, size_t pathCapacity, const char *initialFileName) = 0;
		virtual bool PromptOpenFile(VirtualDirectory_t virtualDirectory, char *path, size_t &outPathLength, size_t pathCapacity) = 0;
		virtual bool ValidateFilePath(const char *path, size_t pathLen) const = 0;

		static HostFileSystem *GetInstance();
		static void SetInstance(HostFileSystem *instance);

	private:
		static HostFileSystem *ms_instance;
	};
}
