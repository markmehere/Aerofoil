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
	GpIOStream *OpenFileNested(PortabilityLayer::VirtualDirectory_t virtualDirectory, char const* const* paths, size_t numPaths, bool writeAccess, GpFileCreationDisposition_t createDisposition) override;
	bool DeleteFile(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *path, bool &existed) override;
	PortabilityLayer::HostDirectoryCursor *ScanDirectoryNested(PortabilityLayer::VirtualDirectory_t virtualDirectory, char const* const* paths, size_t numPaths) override;

	bool ValidateFilePath(const char *path, size_t sz) const override;
	bool ValidateFilePathUnicodeChar(uint32_t ch) const override;

	bool IsVirtualDirectoryLooseResources(PortabilityLayer::VirtualDirectory_t virtualDir) const override;

	void SetMainThreadRelay(IGpThreadRelay *relay) override;
	void SetDelayCallback(DelayCallback_t delayCallback) override;

	const wchar_t *GetBasePath() const;

	static GpFileSystem_Win32 *GetInstance();

private:
	bool ResolvePath(PortabilityLayer::VirtualDirectory_t virtualDirectory, char const* const* paths, size_t numPaths, wchar_t *outPath);

	std::wstring m_prefsDir;
	std::wstring m_scoresDir;
	std::wstring m_packagedDir;
	std::wstring m_housesDir;
	std::wstring m_logsDir;
	std::wstring m_userHousesDir;
	std::wstring m_userSavesDir;
	std::wstring m_resourcesDir;
	std::wstring m_fontCacheDir;
	wchar_t m_executablePath[MAX_PATH];

	static GpFileSystem_Win32 ms_instance;
};
