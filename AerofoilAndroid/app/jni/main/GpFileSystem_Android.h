#pragma once

#include "IGpFileSystem.h"

#include "GpCoreDefs.h"

#include <jni.h>
#include <string>

struct IGpMutex;

class GpFileSystem_Android final : public IGpFileSystem
{
public:
	GpFileSystem_Android();
	~GpFileSystem_Android();

	void InitJNI();
	void ShutdownJNI();

	bool FileExists(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *path) override;
	bool FileLocked(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *path, bool &exists) override;
	GpIOStream *OpenFileNested(PortabilityLayer::VirtualDirectory_t virtualDirectory, char const* const* subPaths, size_t numSubPaths, bool writeAccess, GpFileCreationDisposition_t createDisposition) override;
	bool DeleteFile(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *path, bool &existed) override;
	IGpDirectoryCursor *ScanDirectoryNested(PortabilityLayer::VirtualDirectory_t virtualDirectory, char const* const* paths, size_t numPaths) override;

	bool ValidateFilePath(const char *path, size_t pathLen) const override;
	bool ValidateFilePathUnicodeChar(uint32_t ch) const override;

	void SetDelayCallback(DelayCallback_t delayCallback) override;

	bool OpenGithub() const override;
	bool ShowInstructions() const override;

	static GpFileSystem_Android *GetInstance();

private:
	IGpDirectoryCursor *ScanAssetDirectory(PortabilityLayer::VirtualDirectory_t virtualDirectory, char const* const* paths, size_t numPaths);
	IGpDirectoryCursor *ScanStorageDirectory(PortabilityLayer::VirtualDirectory_t virtualDirectory, char const* const* paths, size_t numPaths);

	bool ResolvePath(PortabilityLayer::VirtualDirectory_t virtualDirectory, char const* const* paths, size_t numPaths, std::string &resolution, bool &isAsset);

	DelayCallback_t m_delayCallback;

	jobject m_activity;
	jmethodID m_scanAssetDirectoryMID;
	jmethodID m_openGithubMID;
	jmethodID m_showInstructionsMID;

	static GpFileSystem_Android ms_instance;
};
