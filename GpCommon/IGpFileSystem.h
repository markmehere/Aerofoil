#pragma once

#include "GpFileCreationDisposition.h"
#include "CoreDefs.h"
#include "VirtualDirectory.h"

#include <stdint.h>
#include <stddef.h>

class GpIOStream;
struct IGpDirectoryCursor;

struct IGpFileSystem
{
public:
	typedef void(*DelayCallback_t)(uint32_t ticks);

	virtual bool FileExists(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *path) = 0;
	virtual bool FileLocked(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *path, bool &exists) = 0;
	virtual GpIOStream *OpenFileNested(PortabilityLayer::VirtualDirectory_t virtualDirectory, char const* const* subPaths, size_t numSubPaths, bool writeAccess, GpFileCreationDisposition_t createDisposition) = 0;
	GP_ASYNCIFY_PARANOID_VIRTUAL bool DeleteFile(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *path, bool &existed) GP_ASYNCIFY_PARANOID_PURE;
	virtual IGpDirectoryCursor *ScanDirectoryNested(PortabilityLayer::VirtualDirectory_t virtualDirectory, char const* const* paths, size_t numPaths) = 0;

	virtual bool ValidateFilePath(const char *path, size_t pathLen) const = 0;
	virtual bool ValidateFilePathUnicodeChar(uint32_t ch) const = 0;

	virtual void SetDelayCallback(DelayCallback_t delayCallback) = 0;

	// Helpers
	GpIOStream *OpenFile(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *path, bool writeAccess, GpFileCreationDisposition_t createDisposition);
	IGpDirectoryCursor *ScanDirectory(PortabilityLayer::VirtualDirectory_t virtualDirectory);
	virtual bool OpenGithub() const = 0;
	virtual bool ShowInstructions() const = 0;
};

inline GpIOStream *IGpFileSystem::OpenFile(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *path, bool writeAccess, GpFileCreationDisposition_t createDisposition)
{
	return this->OpenFileNested(virtualDirectory, &path, 1, writeAccess, createDisposition);
}

inline IGpDirectoryCursor *IGpFileSystem::ScanDirectory(PortabilityLayer::VirtualDirectory_t virtualDirectory)
{
	return this->ScanDirectoryNested(virtualDirectory, nullptr, 0);
}
