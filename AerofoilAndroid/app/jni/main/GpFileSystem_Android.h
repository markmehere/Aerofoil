#pragma once

#include "HostFileSystem.h"

#include "GpCoreDefs.h"

class GpFileSystem_Android final : public PortabilityLayer::HostFileSystem
{
public:
	GpFileSystem_Android();

	bool FileExists(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *path) override;
	bool FileLocked(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *path, bool *exists) override;
	GpIOStream *OpenFile(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *path, bool writeAccess, GpFileCreationDisposition_t createDisposition) override;
	bool DeleteFile(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *path, bool &existed) override;
	PortabilityLayer::HostDirectoryCursor *ScanDirectory(PortabilityLayer::VirtualDirectory_t virtualDirectory) override;

	bool ValidateFilePath(const char *path, size_t sz) const override;
	bool ValidateFilePathUnicodeChar(uint32_t ch) const override;

	static GpFileSystem_Android *GetInstance();

private:
	bool ResolvePath(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *path, wchar_t *outPath);

	static GpFileSystem_Android ms_instance;
};
