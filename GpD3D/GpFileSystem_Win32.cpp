#include "GpFileSystem_Win32.h"
#include "GpFileStream_Win32.h"
#include "GpWindows.h"
#include "GpMemoryBuffer.h"

#include <string>
#include <Shlwapi.h>
#include <ShlObj.h>

GpFileSystem_Win32::GpFileSystem_Win32()
{
	PWSTR docsPath;
	if (!FAILED(SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_DEFAULT, nullptr, &docsPath)))
	{
		try
		{
			m_prefsDir = docsPath;
		}
		catch(...)
		{
			CoTaskMemFree(docsPath);
			throw;
		}

		m_prefsDir.append(L"\\GlidePort");

		CreateDirectoryW(m_prefsDir.c_str(), nullptr);
		m_prefsDir.append(L"\\");
	}
}

bool GpFileSystem_Win32::FileExists(PortabilityLayer::EVirtualDirectory virtualDirectory, const char *path)
{
	wchar_t winPath[MAX_PATH + 1];

	if (!ResolvePath(virtualDirectory, path, winPath))
		return false;

	return PathFileExistsW(winPath) != 0;
}

PortabilityLayer::IOStream *GpFileSystem_Win32::OpenFile(PortabilityLayer::EVirtualDirectory virtualDirectory, const char *path, bool writeAccess, bool create)
{
	wchar_t winPath[MAX_PATH + 1];

	if (!ResolvePath(virtualDirectory, path, winPath))
		return false;

	const DWORD desiredAccess = writeAccess ? (GENERIC_WRITE | GENERIC_READ) : GENERIC_READ;
	const DWORD creationDisposition = create ? OPEN_ALWAYS : OPEN_EXISTING;

	HANDLE h = CreateFileW(winPath, desiredAccess, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

	return new GpFileStream_Win32(h, true, writeAccess, true);
}

GpFileSystem_Win32 *GpFileSystem_Win32::GetInstance()
{
	return &ms_instance;
}

bool GpFileSystem_Win32::ResolvePath(PortabilityLayer::EVirtualDirectory virtualDirectory, const char *path, wchar_t *outPath)
{
	const wchar_t *baseDir = nullptr;

	switch (virtualDirectory)
	{
	case PortabilityLayer::EVirtualDirectory_ApplicationData:
		baseDir = L"D:\\Source Code\\GlidePort\\Packaged\\";
		break;
	case PortabilityLayer::EVirtualDirectory_Prefs:
		baseDir = m_prefsDir.c_str();
		break;
	default:
		return false;
	}

	if (baseDir == nullptr)
		return false;

	const size_t baseDirLen = wcslen(baseDir);
	const size_t pathLen = strlen(path);

	if (baseDirLen >= MAX_PATH || MAX_PATH - baseDirLen < pathLen)
		return false;

	memcpy(outPath, baseDir, sizeof(wchar_t) * baseDirLen);
	for (size_t i = 0; i < pathLen; i++)
		outPath[baseDirLen + i] = static_cast<wchar_t>(path[i]);

	outPath[baseDirLen + pathLen] = static_cast<wchar_t>(0);

	return true;
}

GpFileSystem_Win32 GpFileSystem_Win32::ms_instance;
