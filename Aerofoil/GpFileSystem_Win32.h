#pragma once

#include "HostFileSystem.h"

#include "GpCoreDefs.h"
#include "GpWindows.h"

#include <string>

class GpFileSystem_Win32 final : public PortabilityLayer::HostFileSystem
{
public:
	GpFileSystem_Win32();

	bool FileExists(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *path) override;
	bool FileLocked(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *path, bool *exists) override;
	GpIOStream *OpenFile(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *path, bool writeAccess, GpFileCreationDisposition_t createDisposition) override;
	bool DeleteFile(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *path, bool &existed) override;
	PortabilityLayer::HostDirectoryCursor *ScanDirectory(PortabilityLayer::VirtualDirectory_t virtualDirectory) override;

	bool ValidateFilePath(const char *path, size_t sz) const override;
	bool ValidateFilePathUnicodeChar(uint32_t ch) const override;

	const wchar_t *GetBasePath() const;

	static GpFileSystem_Win32 *GetInstance();

private:
	bool ResolvePath(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *path, wchar_t *outPath);

	std::wstring m_prefsDir;
	std::wstring m_scoresDir;
	std::wstring m_packagedDir;
	std::wstring m_housesDir;
	std::wstring m_logsDir;
	std::wstring m_userHousesDir;
	std::wstring m_userSavesDir;
	std::wstring m_resourcesDir;
	wchar_t m_executablePath[MAX_PATH];

	static GpFileSystem_Win32 ms_instance;
};
