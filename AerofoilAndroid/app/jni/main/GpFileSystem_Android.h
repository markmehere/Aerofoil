#pragma once

#include "HostFileSystem.h"

#include "GpCoreDefs.h"

#include <jni.h>
#include <string>

class GpFileSystem_Android final : public PortabilityLayer::HostFileSystem
{
public:
	GpFileSystem_Android();
	~GpFileSystem_Android();

	void InitJNI();
	void ShutdownJNI();

	bool FileExists(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *path) override;
	bool FileLocked(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *path, bool *exists) override;
	GpIOStream *OpenFileNested(PortabilityLayer::VirtualDirectory_t virtualDirectory, char const* const* subPaths, size_t numSubPaths, bool writeAccess, GpFileCreationDisposition_t createDisposition) override;
	bool DeleteFile(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *path, bool &existed) override;
	PortabilityLayer::HostDirectoryCursor *ScanDirectoryNested(PortabilityLayer::VirtualDirectory_t virtualDirectory, char const* const* paths, size_t numPaths) override;

	bool ValidateFilePath(const char *path, size_t pathLen) const override;
	bool ValidateFilePathUnicodeChar(uint32_t ch) const override;

	bool IsVirtualDirectoryLooseResources(PortabilityLayer::VirtualDirectory_t virtualDir) const override;

	void SetMainThreadRelay(IGpThreadRelay *relay) override;

	static GpFileSystem_Android *GetInstance();

private:
	struct ScanDirectoryNestedContext
	{
		GpFileSystem_Android *m_this;

		PortabilityLayer::HostDirectoryCursor *m_returnValue;
		PortabilityLayer::VirtualDirectory_t m_virtualDirectory;
		char const *const *m_paths;
		size_t m_numPaths;
	};

	static void ScanDirectoryNestedThunk(void *context);
	PortabilityLayer::HostDirectoryCursor *ScanDirectoryNestedInternal(PortabilityLayer::VirtualDirectory_t virtualDirectory, char const* const* paths, size_t numPaths);

	PortabilityLayer::HostDirectoryCursor *ScanAssetDirectory(PortabilityLayer::VirtualDirectory_t virtualDirectory, char const* const* paths, size_t numPaths);
	PortabilityLayer::HostDirectoryCursor *ScanStorageDirectory(PortabilityLayer::VirtualDirectory_t virtualDirectory, char const* const* paths, size_t numPaths);

	bool ResolvePathInDownloadsDirectory(PortabilityLayer::VirtualDirectory_t virtualDirectory, char const* const* paths, size_t numPaths, std::string &resolution, bool &isAsset);
	bool ResolvePath(PortabilityLayer::VirtualDirectory_t virtualDirectory, char const* const* paths, size_t numPaths, std::string &resolution, bool &isAsset);

	IGpThreadRelay *m_relay;

	jobject m_activity;
	jmethodID m_scanAssetDirectoryMID;
	jmethodID m_selectSourceExportPathMID;

	static GpFileSystem_Android ms_instance;
};
