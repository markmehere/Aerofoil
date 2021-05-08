#define _LARGEFILE64_SOURCE
#include "GpFileSystem_Web.h"
#include "GpIOStream.h"
#include "IGpDirectoryCursor.h"
#include "IGpSystemServices.h"
#include "IGpMutex.h"
#include "VirtualDirectory.h"

#include "PLDrivers.h"

#include "SDL2/SDL.h"
#include "SDL2/SDL_rwops.h"

#include <string>
#include <vector>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <emscripten.h>

#include "UTF8.h"

#if defined(__CYGWIN__) || defined(__EMSCRIPTEN__)
typedef off_t off64_t;
#define fstat64 fstat
#define fseek64 fseek
#define ftruncate64 ftruncate
#define stat64 stat
#endif

EM_JS(void, FlushFileSystem, (), {
	Asyncify.handleSleep(wakeUp => {
		FS.syncfs(false, function (err) {
			assert(!err);
			wakeUp();
		});
	});
});

EM_JS(void, DownloadAndDeleteFile, (const char *fileNamePtr, const char *prettyNamePtr), {
	var fileName = UTF8ToString(fileNamePtr);
	var prettyName = UTF8ToString(prettyNamePtr);
	console.log("Flush download of file " + fileName + " as " + prettyName);

	var mimeType = "application/octet-stream";
	if (prettyName.endsWith(".bin"))
		mimeType = "application/macbinary";
	else if (prettyName.endsWith(".gpf"))
		mimeType = "application/zip";

	var byteArray = FS.readFile(fileName, { encoding: "binary" });
	var blob = new Blob([byteArray], { type: mimeType });
	saveAs(blob, prettyName);
});


class GpDirectoryCursor_Web final : public IGpDirectoryCursor
{
public:
	explicit GpDirectoryCursor_Web(DIR *dir, const std::string &prefix);
	~GpDirectoryCursor_Web();

	bool GetNext(const char *&outFileName) override;
	void Destroy() override;

private:
	DIR *m_dir;
	std::string m_prefix;
	std::string m_decodedFileName;
};

GpDirectoryCursor_Web::GpDirectoryCursor_Web(DIR *dir, const std::string &prefix)
	: m_dir(dir)
	, m_prefix(prefix)
{
}

GpDirectoryCursor_Web::~GpDirectoryCursor_Web()
{
	closedir(m_dir);
}

bool GpDirectoryCursor_Web::GetNext(const char *&outFileName)
{
	const size_t prefixLength = m_prefix.size();

	for (;;)
	{
		struct dirent *dir = readdir(m_dir);
		if (!dir)
			return false;

		const char *fname = dir->d_name;

		const size_t fnameLen = strlen(fname);
		if (fnameLen > prefixLength && (prefixLength == 0 || !memcmp(&m_prefix[0], fname, prefixLength)))
		{
			const char *encodedResult = fname + prefixLength;

			m_decodedFileName.clear();
			for (size_t i = 0; encodedResult[i] != 0; i++)
			{
				char c = encodedResult[i];
				if (c == '%')
				{
					char highNibble = encodedResult[i + 1];
					if ((highNibble >= '0' && highNibble <= '9') || (highNibble >= 'a' && highNibble <= 'f') || (highNibble >= 'A' && highNibble <= 'F'))
					{
						char lowNibble = encodedResult[i + 2];

						if ((lowNibble >= '0' && lowNibble <= '9') || (lowNibble >= 'a' && lowNibble <= 'f') || (lowNibble >= 'A' && lowNibble <= 'F'))
						{
							bool failedNibble = false;
							char nibbles[2] = { highNibble, lowNibble };
							int decNibbles[2];
							for (int ni = 0; ni < 2; ni++)
							{
								char nc = nibbles[ni];
								if (nc >= '0' && nc <= '9')
									decNibbles[ni] = nc - '0';
								else if (nc >= 'a' && nc <= 'f')
									decNibbles[ni] = 0xa + (nc - 'a');
								else if (nc >= 'A' && nc <= 'F')
									decNibbles[ni] = 0xa + (nc - 'A');
								else
									failedNibble = true;
							}
							
							if (!failedNibble)
							{
								c = static_cast<char>((decNibbles[0] << 4) + decNibbles[1]);
								i += 2;
							}
						}
					}
				}

				m_decodedFileName += c;
			}
			
			outFileName = m_decodedFileName.c_str();
			return true;
		}
	}
	return true;
}

void GpDirectoryCursor_Web::Destroy()
{
	delete this;
}

class GpFileStream_Web_StaticMemFile final : public GpIOStream
{
public:
	GpFileStream_Web_StaticMemFile(const unsigned char *bytes, size_t size);
	~GpFileStream_Web_StaticMemFile();

	size_t Read(void *bytesOut, size_t size) override;
	size_t Write(const void *bytes, size_t size) override;
	bool IsSeekable() const override;
	bool IsReadOnly() const override;
	bool IsWriteOnly() const override;
	bool SeekStart(GpUFilePos_t loc) override;
	bool SeekCurrent(GpFilePos_t loc) override;
	bool SeekEnd(GpUFilePos_t loc) override;
	GpUFilePos_t Size() const override;
	GpUFilePos_t Tell() const override;
	void GP_ASYNCIFY_PARANOID_NAMED(Close)() override;
	void Flush() override;

private:
	const unsigned char *m_bytes;
	size_t m_offset;
	size_t m_size;
};

GpFileStream_Web_StaticMemFile::GpFileStream_Web_StaticMemFile(const unsigned char *bytes, size_t size)
	: m_bytes(bytes)
	, m_size(size)
	, m_offset(0)
{
}

GpFileStream_Web_StaticMemFile::~GpFileStream_Web_StaticMemFile()
{
}

size_t GpFileStream_Web_StaticMemFile::Read(void *bytesOut, size_t size)
{
	size_t available = m_size - m_offset;
	size = std::min(size, available);
	
	memcpy(bytesOut, m_bytes + m_offset, size);
	m_offset += size;
	return size;
}

size_t GpFileStream_Web_StaticMemFile::Write(const void *bytes, size_t size)
{
	return 0;
}

bool GpFileStream_Web_StaticMemFile::IsSeekable() const
{
	return true;
}

bool GpFileStream_Web_StaticMemFile::IsReadOnly() const
{
	return true;
}

bool GpFileStream_Web_StaticMemFile::IsWriteOnly() const
{
	return false;
}

bool GpFileStream_Web_StaticMemFile::SeekStart(GpUFilePos_t loc)
{
	if (loc > m_size)
		return false;

	m_offset = static_cast<size_t>(loc);
	return true;
}

bool GpFileStream_Web_StaticMemFile::SeekCurrent(GpFilePos_t loc)
{
	GpFilePos_t minOffset = -static_cast<GpFilePos_t>(m_offset);
	GpFilePos_t maxOffset = static_cast<GpFilePos_t>(m_size - m_offset);
	
	if (loc < minOffset || loc > maxOffset)
		return false;

	m_offset = static_cast<size_t>(static_cast<GpFilePos_t>(m_offset) + loc);
	return true;
}

bool GpFileStream_Web_StaticMemFile::SeekEnd(GpUFilePos_t loc)
{
	if (loc > m_size)
		return false;

	m_offset = m_size - loc;
	return true;
}

GpUFilePos_t GpFileStream_Web_StaticMemFile::Size() const
{
	return m_size;
}

GpUFilePos_t GpFileStream_Web_StaticMemFile::Tell() const
{
	return m_offset;
}

void GpFileStream_Web_StaticMemFile::GP_ASYNCIFY_PARANOID_NAMED(Close)()
{
	delete this;
}

void GpFileStream_Web_StaticMemFile::Flush()
{
}


class GpFileStream_Web_File final : public GpIOStream
{
public:
	GpFileStream_Web_File(FILE *f, const std::string &filePath, const std::string &prettyName, bool readOnly, bool writeOnly, bool synchronizeOnClose, bool isIDB);
	~GpFileStream_Web_File();

	size_t Read(void *bytesOut, size_t size) override;
	size_t Write(const void *bytes, size_t size) override;
	bool IsSeekable() const override;
	bool IsReadOnly() const override;
	bool IsWriteOnly() const override;
	bool SeekStart(GpUFilePos_t loc) override;
	bool SeekCurrent(GpFilePos_t loc) override;
	bool SeekEnd(GpUFilePos_t loc) override;
	GpUFilePos_t Size() const override;
	GpUFilePos_t Tell() const override;
	void GP_ASYNCIFY_PARANOID_NAMED(Close)() override;
	void Flush() override;

private:
	FILE *m_f;
	std::string m_filePath;
	std::string m_prettyName;
	bool m_seekable;
	bool m_isReadOnly;
	bool m_isWriteOnly;
	bool m_synchronizeOnClose;
	bool m_isIDB;
};


GpFileStream_Web_File::GpFileStream_Web_File(FILE *f, const std::string &filePath, const std::string &prettyName, bool readOnly, bool writeOnly, bool synchronizeOnClose, bool isIDB)
	: m_f(f)
	, m_isReadOnly(readOnly)
	, m_isWriteOnly(writeOnly)
	, m_synchronizeOnClose(synchronizeOnClose)
	, m_isIDB(isIDB)
	, m_filePath(filePath)
	, m_prettyName(prettyName)
{
	m_seekable = (fseek(m_f, 0, SEEK_CUR) == 0);
}

GpFileStream_Web_File::~GpFileStream_Web_File()
{
	fclose(m_f);

	if (m_synchronizeOnClose)
	{
		if (m_isIDB)
			GpFileSystem_Web::MarkFSStateDirty();
		else
			GpFileSystem_Web::SyncDownloadFile(m_filePath, m_prettyName);
	}
}

size_t GpFileStream_Web_File::Read(void *bytesOut, size_t size)
{
	if (m_isWriteOnly)
		return 0;
	return fread(bytesOut, 1, size, m_f);
}

size_t GpFileStream_Web_File::Write(const void *bytes, size_t size)
{
	if (m_isReadOnly)
		return 0;
	return fwrite(bytes, 1, size, m_f);
}

bool GpFileStream_Web_File::IsSeekable() const
{
	return m_seekable;
}

bool GpFileStream_Web_File::IsReadOnly() const
{
	return m_isReadOnly;
}

bool GpFileStream_Web_File::IsWriteOnly() const
{
	return m_isWriteOnly;
}

bool GpFileStream_Web_File::SeekStart(GpUFilePos_t loc)
{
	if (!m_seekable)
		return false;

	fflush(m_f);
	return fseek64(m_f, static_cast<off64_t>(loc), SEEK_SET) >= 0;
}

bool GpFileStream_Web_File::SeekCurrent(GpFilePos_t loc)
{
	if (!m_seekable)
		return false;

	fflush(m_f);
	return fseek64(m_f, static_cast<off64_t>(loc), SEEK_CUR) >= 0;
}

bool GpFileStream_Web_File::SeekEnd(GpUFilePos_t loc)
{
	if (!m_seekable)
		return false;

	fflush(m_f);
	return fseek64(m_f, -static_cast<off64_t>(loc), SEEK_END) >= 0;
}

GpUFilePos_t GpFileStream_Web_File::Size() const
{
	fflush(m_f);

	struct stat64 s;
	if (fstat64(fileno(m_f), &s) < 0)
		return 0;

	return static_cast<GpUFilePos_t>(s.st_size);
}

GpUFilePos_t GpFileStream_Web_File::Tell() const
{
	return static_cast<GpUFilePos_t>(ftell(m_f));
}

void GpFileStream_Web_File::GP_ASYNCIFY_PARANOID_NAMED(Close)()
{
	this->~GpFileStream_Web_File();
	free(this);
}

void GpFileStream_Web_File::Flush()
{
	fflush(m_f);
}


bool GpFileSystem_Web::ms_fsStateDirty;


bool GpFileSystem_Web::ResolvePath(PortabilityLayer::VirtualDirectory_t virtualDirectory, char const* const* paths, size_t numPaths, bool trailingSlash, std::string &resolution, bool &outIsIDB)
{
	const char *pathAppend = nullptr;
	const std::string *rootPath = nullptr;
	std::string unsanitized;
	bool isIDB = false;

	switch (virtualDirectory)
	{
	case PortabilityLayer::VirtualDirectories::kApplicationData:
		unsanitized = std::string("Packaged");
		break;
	case PortabilityLayer::VirtualDirectories::kGameData:
		unsanitized = std::string("Packaged/Houses");
		break;
	case PortabilityLayer::VirtualDirectories::kFonts:
		unsanitized = std::string("Resources");
		break;
	case PortabilityLayer::VirtualDirectories::kHighScores:
		pathAppend = "HighScores";
		rootPath = &m_basePath;
		isIDB = true;
		break;
	case PortabilityLayer::VirtualDirectories::kUserData:
		pathAppend = "Houses";
		rootPath = &m_basePath;
		isIDB = true;
		break;
	case PortabilityLayer::VirtualDirectories::kUserSaves:
		pathAppend = "SavedGames";
		rootPath = &m_basePath;
		isIDB = true;
		break;
	case PortabilityLayer::VirtualDirectories::kPrefs:
		pathAppend = "Prefs";
		rootPath = &m_basePath;
		isIDB = true;
		break;
	case PortabilityLayer::VirtualDirectories::kSourceExport:
		pathAppend = "Export";
		rootPath = &m_exportPath;
		isIDB = false;
		break;
	default:
		return false;
	};

	if (pathAppend)
	{
		unsanitized = pathAppend;
		
		for (size_t i = 0; i < numPaths; i++)
		{
			unsanitized += "/";
			unsanitized += paths[i];
		}
		
		if (trailingSlash)
			unsanitized += "/";
		
		std::string sanitized;
		for (size_t i = 0; i < unsanitized.size(); i++)
		{
			char c = unsanitized[i];
			if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')
				sanitized += c;
			else
			{
				const char *nibbles = "0123456789abcdef";
				char subPath[4];
				subPath[0] = '%';
				subPath[1] = nibbles[(c >> 4) & 0xf];
				subPath[2] = nibbles[c & 0xf];
				subPath[3] = 0;

				sanitized += subPath;
			}
		}

		resolution = (*rootPath) + "/" + sanitized;
	}
	else
	{
		std::string sanitized = m_basePath + unsanitized;
		
		for (size_t i = 0; i < numPaths; i++)
		{
			sanitized += "/";
			sanitized += paths[i];
		}

		resolution = sanitized;
	}

	outIsIDB = isIDB;
	return true;
}

GpFileSystem_Web::GpFileSystem_Web()
	: m_delayCallback(nullptr)
{
}

GpFileSystem_Web::~GpFileSystem_Web()
{
}

void GpFileSystem_Web::Init()
{
	m_prefsPath = "/aerofoil";
	m_exportPath = "/aerofoil_memfs";

	char *baseDir = SDL_GetBasePath();
	m_basePath = baseDir;
	SDL_free(baseDir);

	char baseDirSeparator = m_basePath[m_basePath.size() - 1];
	if (m_basePath.size() >= 4 && m_basePath.substr(m_basePath.size() - 4, 3) == "bin")
		m_basePath = m_basePath.substr(0, m_basePath.size() - 4) + "lib" + baseDirSeparator + "aerofoil" + baseDirSeparator;
}

bool GpFileSystem_Web::FileExists(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *path)
{
	if (const GpFileSystem_Web_Resources::FileCatalog *catalog = GetCatalogForVirtualDirectory(virtualDirectory))
	{
		for (size_t i = 0; i < catalog->m_numEntries; i++)
		{
			const GpFileSystem_Web_Resources::FileCatalogEntry &entry = catalog->m_entries[i];
			if (!strcmp(path, entry.m_fileName))
				return true;
		}

		return false;
	}

	std::string resolvedPath;
	bool isIDB = false;
	if (!ResolvePath(virtualDirectory, &path, 1, false, resolvedPath, isIDB))
		return false;

	struct stat s;
	return stat(resolvedPath.c_str(), &s) == 0;
}

bool GpFileSystem_Web::FileLocked(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *path, bool &exists)
{
	if (const GpFileSystem_Web_Resources::FileCatalog *catalog = GetCatalogForVirtualDirectory(virtualDirectory))
	{
		for (size_t i = 0; i < catalog->m_numEntries; i++)
		{
			const GpFileSystem_Web_Resources::FileCatalogEntry &entry = catalog->m_entries[i];
			if (!strcmp(path, entry.m_fileName))
			{
				exists = true;
				return true;
			}
		}

		exists = false;
		return false;
	}

	std::string resolvedPath;
	bool isIDB = false;
	if (!ResolvePath(virtualDirectory, &path, 1, false, resolvedPath, isIDB))
	{
		if (exists)
			exists = false;
		return false;
	}

	int permissions = access(resolvedPath.c_str(), W_OK | F_OK);
	exists = ((permissions & F_OK) != 0);
	return ((permissions & W_OK) != 0);
}

GpIOStream *GpFileSystem_Web::OpenFileNested(PortabilityLayer::VirtualDirectory_t virtualDirectory, char const* const* subPaths, size_t numSubPaths, bool writeAccess, GpFileCreationDisposition_t createDisposition)
{
	if (numSubPaths == 1)
	{
		if (const GpFileSystem_Web_Resources::FileCatalog *catalog = GetCatalogForVirtualDirectory(virtualDirectory))
		{
			for (size_t i = 0; i < catalog->m_numEntries; i++)
			{
				const GpFileSystem_Web_Resources::FileCatalogEntry &entry = catalog->m_entries[i];
				if (!strcmp(subPaths[0], entry.m_fileName))
					return new GpFileStream_Web_StaticMemFile(entry.m_data, entry.m_size);
			}

			return nullptr;
		}
	}

	const char *mode = nullptr;
	bool canWrite = false;
	bool needResetPosition = false;

	switch (createDisposition)
	{
		case GpFileCreationDispositions::kCreateOrOverwrite:
			mode = "wb";
			break;
		case GpFileCreationDispositions::kCreateNew:
			mode = "x+b";
			break;
		case GpFileCreationDispositions::kCreateOrOpen:
			mode = "a+b";
			needResetPosition = true;
			break;
		case GpFileCreationDispositions::kOpenExisting:
			mode = writeAccess ? "r+b" : "rb";
			break;
		case GpFileCreationDispositions::kOverwriteExisting:
			mode = "r+b";
			break;
		default:
			return nullptr;
	};

	std::string resolvedPath;
	bool isIDB = false;
	if (!ResolvePath(virtualDirectory, subPaths, numSubPaths, false, resolvedPath, isIDB))
		return nullptr;

	void *objStorage = malloc(sizeof(GpFileStream_Web_File));
	if (!objStorage)
		return nullptr;

	FILE *f = fopen(resolvedPath.c_str(), mode);
	if (!f)
	{
		free(objStorage);
		return nullptr;
	}

	if (needResetPosition)
		fseek(f, 0, SEEK_SET);

	if (createDisposition == GpFileCreationDispositions::kOverwriteExisting)
	{
		if (ftruncate64(fileno(f), 0) < 0)
		{
			free(objStorage);
			fclose(f);
			return nullptr;
		}
	}

	std::string prettyName;
	if (numSubPaths > 0)
		prettyName = subPaths[numSubPaths - 1];

	return new (objStorage) GpFileStream_Web_File(f, resolvedPath, prettyName, !writeAccess, false, writeAccess, isIDB);
}

bool GpFileSystem_Web::DeleteFile(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *path, bool &existed)
{
	if (const GpFileSystem_Web_Resources::FileCatalog *catalog = GetCatalogForVirtualDirectory(virtualDirectory))
		return false;

	std::string resolvedPath;
	bool isIDB = false;
	if (!ResolvePath(virtualDirectory, &path, 1, false, resolvedPath, isIDB))
	{
		existed = false;
		return false;
	}

	if (unlink(resolvedPath.c_str()) < 0)
	{
		existed = (errno != ENOENT);
		if (existed && isIDB)
			FlushFileSystem();

		return false;
	}
	existed = true;
	return true;
}

bool GpFileSystem_Web::ValidateFilePath(const char *path, size_t length) const
{
	for (size_t i = 0; i < length; i++)
	{
		const char c = path[i];
		if (c >= '0' && c <= '9')
			continue;

		if (c == '_' || c == '.' || c == '\'' || c == '!')
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

bool GpFileSystem_Web::ValidateFilePathUnicodeChar(uint32_t c) const
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

void GpFileSystem_Web::SetDelayCallback(DelayCallback_t delayCallback)
{
	m_delayCallback = delayCallback;
}

GpFileSystem_Web *GpFileSystem_Web::GetInstance()
{
	return &ms_instance;
}

class GpDirectoryCursor_StringList final : public IGpDirectoryCursor
{
public:
	explicit GpDirectoryCursor_StringList(std::vector<std::string> &paths);
	~GpDirectoryCursor_StringList();

	bool GetNext(const char *&outFileName) override;
	void Destroy() override;

private:
	std::vector<std::string> m_paths;
	size_t m_index;
};

GpDirectoryCursor_StringList::GpDirectoryCursor_StringList(std::vector<std::string> &paths)
	: m_index(0)
{
	std::swap(paths, m_paths);
}

GpDirectoryCursor_StringList::~GpDirectoryCursor_StringList()
{
}

bool GpDirectoryCursor_StringList::GetNext(const char *&outFileName)
{
	if (m_index == m_paths.size())
		return false;
	outFileName = m_paths[m_index].c_str();
	m_index++;
	return true;
}

void GpDirectoryCursor_StringList::Destroy()
{
	delete this;
}


IGpDirectoryCursor *GpFileSystem_Web::ScanDirectoryNested(PortabilityLayer::VirtualDirectory_t virtualDirectory, char const* const* paths, size_t numPaths)
{
	if (const GpFileSystem_Web_Resources::FileCatalog *catalog = GetCatalogForVirtualDirectory(virtualDirectory))
		return ScanCatalog(*catalog);

	std::string resolvedPrefix;
	bool isIDB = false;
	if (!ResolvePath(virtualDirectory, paths, numPaths, true, resolvedPrefix, isIDB))
		return nullptr;

	std::string trimmedPrefix = resolvedPrefix.substr(m_prefsPath.size() + 1);

	DIR *d = opendir(m_prefsPath.c_str());
	if (!d)
		return nullptr;

	return new GpDirectoryCursor_Web(d, trimmedPrefix);
}

const GpFileSystem_Web_Resources::FileCatalog *GpFileSystem_Web::GetCatalogForVirtualDirectory(PortabilityLayer::VirtualDirectory_t virtualDirectory)
{
	if (virtualDirectory == PortabilityLayer::VirtualDirectories::kApplicationData)
		return &GpFileSystem_Web_Resources::ApplicationData::GetCatalog();

	if (virtualDirectory == PortabilityLayer::VirtualDirectories::kGameData)
		return &GpFileSystem_Web_Resources::GameData::GetCatalog();

	return nullptr;
}

IGpDirectoryCursor *GpFileSystem_Web::ScanCatalog(const GpFileSystem_Web_Resources::FileCatalog &catalog)
{
	std::vector<std::string> paths;
	for (size_t i = 0; i < catalog.m_numEntries; i++)
		paths.push_back(std::string(catalog.m_entries[i].m_fileName));

	return new GpDirectoryCursor_StringList(paths);
}

void GpFileSystem_Web::MarkFSStateDirty()
{
	ms_fsStateDirty = true;
}

void GpFileSystem_Web::SyncDownloadFile(const std::string &filePath, const std::string &prettyName)
{
	DownloadAndDeleteFile(filePath.c_str(), prettyName.c_str());
}

void GpFileSystem_Web::FlushFS()
{
	if (ms_fsStateDirty)
	{
		ms_fsStateDirty = false;
		FlushFileSystem();
	}
}


#if GP_ASYNCIFY_PARANOID
void GpIOStream::Close()
{
	this->GP_ASYNCIFY_PARANOID_NAMED(Close)();
	GpFileSystem_Web::FlushFS();
}

bool IGpFileSystem::DeleteFile(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *path, bool &existed)
{
	return static_cast<GpFileSystem_Web*>(this)->DeleteFile(virtualDirectory, path, existed);
}

#endif

GpFileSystem_Web GpFileSystem_Web::ms_instance;
