#pragma once

#include "IGpFileSystem.h"

#include "GpCoreDefs.h"
#include "GpWindows.h"
#include "GpString.h"

class GpFileSystem_Win32 final : public IGpFileSystem
{
public:
	explicit GpFileSystem_Win32(IGpAllocator *alloc);

	void Destroy();

	bool FileExists(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *path) override;
	bool FileLocked(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *path, bool &exists) override;
	GpIOStream *OpenFileNested(PortabilityLayer::VirtualDirectory_t virtualDirectory, char const* const* paths, size_t numPaths, bool writeAccess, GpFileCreationDisposition_t createDisposition) override;
	bool DeleteFile(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *path, bool &existed) override;
	IGpDirectoryCursor *ScanDirectoryNested(PortabilityLayer::VirtualDirectory_t virtualDirectory, char const* const* paths, size_t numPaths) override;

	bool ValidateFilePath(const char *path, size_t sz) const override;
	bool ValidateFilePathUnicodeChar(uint32_t ch) const override;

	void SetDelayCallback(DelayCallback_t delayCallback) override;

	const wchar_t *GetBasePath() const;

	static GpFileSystem_Win32 *CreateInstance(IGpAllocator *alloc);
	static GpFileSystem_Win32 *GetInstance();

private:
	bool Init();

	bool ResolvePath(PortabilityLayer::VirtualDirectory_t virtualDirectory, char const* const* paths, size_t numPaths, wchar_t *outPath);

	GpWString m_prefsDir;
	GpWString m_scoresDir;
	GpWString m_packagedDir;
	GpWString m_housesDir;
	GpWString m_logsDir;
	GpWString m_userHousesDir;
	GpWString m_userSavesDir;
	GpWString m_resourcesDir;
	GpWString m_exportDir;
	wchar_t m_executablePath[MAX_PATH];

	IGpAllocator *m_alloc;

	static GpFileSystem_Win32 *ms_instance;
};
