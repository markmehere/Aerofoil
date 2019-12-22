#pragma once

#include "HostFileSystem.h"

#include "GpCoreDefs.h"
#include "GpWindows.h"

#include <string>

class GpFileSystem_Win32 final : public PortabilityLayer::HostFileSystem
{
public:
	GpFileSystem_Win32();

	bool FileExists(PortabilityLayer::EVirtualDirectory virtualDirectory, const char *path) override;
	PortabilityLayer::IOStream *OpenFile(PortabilityLayer::EVirtualDirectory virtualDirectory, const char *path, bool writeAccess, bool create) override;
	PortabilityLayer::HostDirectoryCursor *ScanDirectory(PortabilityLayer::EVirtualDirectory virtualDirectory) override;

	const wchar_t *GetBasePath() const;

	static GpFileSystem_Win32 *GetInstance();

private:
	bool ResolvePath(PortabilityLayer::EVirtualDirectory virtualDirectory, const char *path, wchar_t *outPath);

	std::wstring m_prefsDir;
	std::wstring m_packagedDir;
	std::wstring m_housesDir;
	std::wstring m_resourcesDir;
	wchar_t m_executablePath[MAX_PATH];

	static GpFileSystem_Win32 ms_instance;
};
