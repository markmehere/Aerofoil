#include "GpFileSystem_Win32.h"
#include "GpFileStream_Win32.h"
#include "GpWindows.h"
#include "GpMemoryBuffer.h"
#include "HostDirectoryCursor.h"

#include <string>
#include <Shlwapi.h>
#include <ShlObj.h>
#include <assert.h>

class GpDirectoryCursor_Win32 final : public PortabilityLayer::HostDirectoryCursor
{
public:
	static GpDirectoryCursor_Win32 *Create(const HANDLE &handle, const WIN32_FIND_DATAW &findData);

	bool GetNext(const char *&outFileName) override;
	void Destroy() override;

private:
	GpDirectoryCursor_Win32(const HANDLE &handle, const WIN32_FIND_DATAW &findData);
	~GpDirectoryCursor_Win32();

	HANDLE m_handle;
	WIN32_FIND_DATAW m_findData;
	char m_chars[MAX_PATH + 1];
	bool m_haveNext;
};

GpDirectoryCursor_Win32 *GpDirectoryCursor_Win32::Create(const HANDLE &handle, const WIN32_FIND_DATAW &findData)
{
	void *storage = malloc(sizeof(GpDirectoryCursor_Win32));
	if (!storage)
		return nullptr;

	return new (storage) GpDirectoryCursor_Win32(handle, findData);
}

bool GpDirectoryCursor_Win32::GetNext(const char *&outFileName)
{
	while (m_haveNext)
	{
		bool haveResult = false;

		bool hasInvalidChars = false;
		for (const wchar_t *fnameScan = m_findData.cFileName; *fnameScan; fnameScan++)
		{
			const int32_t asInt = static_cast<int32_t>(*fnameScan);
			if (asInt < 1 || asInt >= 128)
			{
				hasInvalidChars = true;
				break;
			}
		}

		if (!hasInvalidChars && wcscmp(m_findData.cFileName, L".") && wcscmp(m_findData.cFileName, L".."))
		{
			const size_t len = wcslen(m_findData.cFileName);

			haveResult = true;

			for (size_t i = 0; i <= len; i++)
				m_chars[i] = static_cast<char>(m_findData.cFileName[i]);
		}

		m_haveNext = (FindNextFileW(m_handle, &m_findData) != FALSE);

		if (haveResult)
		{
			outFileName = m_chars;
			return true;
		}
	}

	return false;
}

void GpDirectoryCursor_Win32::Destroy()
{
	this->~GpDirectoryCursor_Win32();
	free(this);
}

GpDirectoryCursor_Win32::GpDirectoryCursor_Win32(const HANDLE &handle, const WIN32_FIND_DATAW &findData)
	: m_handle(handle)
	, m_findData(findData)
	, m_haveNext(true)
{
}

GpDirectoryCursor_Win32::~GpDirectoryCursor_Win32()
{
	FindClose(m_handle);
}

GpFileSystem_Win32::GpFileSystem_Win32()
{
	// GP TODO: This shouldn't be static init since it allocates memory
	m_executablePath[0] = 0;

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

	DWORD modulePathSize = GetModuleFileNameW(nullptr, m_executablePath, MAX_PATH);
	if (modulePathSize == MAX_PATH || modulePathSize == 0)
		m_executablePath[0] = 0;

	size_t currentPathLength = wcslen(m_executablePath);

	for (;;)
	{
		while (currentPathLength > 0 && m_executablePath[currentPathLength - 1] != '\\')
			currentPathLength--;

		m_executablePath[currentPathLength] = 0;

		if (currentPathLength + 11 > MAX_PATH)
		{
			// "Resources" append is a longer path than the executable
			continue;
		}

		if (wcscat_s(m_executablePath, L"Resources"))
		{
			currentPathLength = 0;
			break;
		}

		if (PathFileExistsW(m_executablePath) && PathIsDirectoryW(m_executablePath))
		{
			m_executablePath[currentPathLength] = 0;
			break;
		}
		else
			currentPathLength--;
	}

	if (currentPathLength > 0)
	{
		m_packagedDir = std::wstring(m_executablePath) + L"Packaged\\";
		m_housesDir = std::wstring(m_executablePath) + L"Packaged\\Houses\\";
		m_resourcesDir = std::wstring(m_executablePath) + L"Resources\\";
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
	if (h == INVALID_HANDLE_VALUE)
		return false;

	return new GpFileStream_Win32(h, true, writeAccess, true);
}

PortabilityLayer::HostDirectoryCursor *GpFileSystem_Win32::ScanDirectory(PortabilityLayer::EVirtualDirectory virtualDirectory)
{
	wchar_t winPath[MAX_PATH + 2];

	if (!ResolvePath(virtualDirectory, "*", winPath))
		return nullptr;

	WIN32_FIND_DATAW findData;
	HANDLE ff = FindFirstFileW(winPath, &findData);

	if (ff == INVALID_HANDLE_VALUE)
		return nullptr;

	return GpDirectoryCursor_Win32::Create(ff, findData);
}

const wchar_t *GpFileSystem_Win32::GetBasePath() const
{
	return m_executablePath;
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
		baseDir = m_packagedDir.c_str();
		break;
	case PortabilityLayer::EVirtualDirectory_GameData:
		baseDir = m_housesDir.c_str();
		break;
	case PortabilityLayer::EVirtualDirectory_Prefs:
		baseDir = m_prefsDir.c_str();
		break;
	case PortabilityLayer::EVirtualDirectory_Fonts:
		baseDir = m_resourcesDir.c_str();
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
