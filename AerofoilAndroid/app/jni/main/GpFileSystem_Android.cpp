#include "GpFileSystem_Android.h"

GpFileSystem_Android::GpFileSystem_Android()
{
}

bool GpFileSystem_Android::FileExists(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *path)
{
	return false;
}

bool GpFileSystem_Android::FileLocked(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *path, bool *exists)
{
	return false;
}

GpIOStream *GpFileSystem_Android::OpenFile(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *path, bool writeAccess, GpFileCreationDisposition_t createDisposition)
{
	return nullptr;
}

bool GpFileSystem_Android::DeleteFile(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *path, bool &existed)
{
	existed = false;
	return false;
}

PortabilityLayer::HostDirectoryCursor *GpFileSystem_Android::ScanDirectory(PortabilityLayer::VirtualDirectory_t virtualDirectory)
{
	return nullptr;
}

bool GpFileSystem_Android::ValidateFilePath(const char *path, size_t length) const
{
	for (size_t i = 0; i < length; i++)
	{
		const char c = path[i];
		if (c >= '0' && c <= '9')
			continue;

		if (c == '_' || c == '.' || c == '\'')
			continue;

		if (c == ' ' && i != 0 && i != length - 1)
			continue;

		if (c >= 'a' && c <= 'z')
			continue;

		if (c >= 'A' && c <= 'Z')
			continue;

		return false;
	}

	return true;
}

bool GpFileSystem_Android::ValidateFilePathUnicodeChar(uint32_t c) const
{
	if (c >= '0' && c <= '9')
		return true;

	if (c == '_' || c == '\'')
		return true;

	if (c == ' ')
		return true;

	if (c >= 'a' && c <= 'z')
		return true;

	if (c >= 'A' && c <= 'Z')
		return true;

	return false;
}

GpFileSystem_Android *GpFileSystem_Android::GetInstance()
{
	return &ms_instance;
}

GpFileSystem_Android GpFileSystem_Android::ms_instance;
