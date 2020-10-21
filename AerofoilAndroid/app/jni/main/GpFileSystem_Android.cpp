#define _LARGEFILE64_SOURCE
#include "GpFileSystem_Android.h"
#include "GpIOStream.h"
#include "HostDirectoryCursor.h"
#include "IGpThreadRelay.h"
#include "VirtualDirectory.h"

#include "SDL.h"
#include "SDL_rwops.h"

#include <string>
#include <vector>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <jni.h>
#include "UTF8.h"



class GpFileStream_SDLRWops final : public GpIOStream
{
public:
	GpFileStream_SDLRWops(SDL_RWops *f, bool readOnly, bool writeOnly);
	~GpFileStream_SDLRWops();

	size_t Read(void *bytesOut, size_t size) override;
	size_t Write(const void *bytes, size_t size) override;
	bool IsSeekable() const override;
	bool IsReadOnly() const override;
	bool IsWriteOnly() const override;
	bool SeekStart(GpUFilePos_t loc) override;
	bool SeekCurrent(GpFilePos_t loc) override;
	bool SeekEnd(GpUFilePos_t loc) override;
	bool Truncate(GpUFilePos_t loc) override;
	GpUFilePos_t Size() const override;
	GpUFilePos_t Tell() const override;
	void Close() override;
	void Flush() override;

private:
	SDL_RWops *m_rw;
	bool m_isReadOnly;
	bool m_isWriteOnly;
};


GpFileStream_SDLRWops::GpFileStream_SDLRWops(SDL_RWops *f, bool readOnly, bool writeOnly)
	: m_rw(f)
	, m_isReadOnly(readOnly)
	, m_isWriteOnly(writeOnly)
{
}


GpFileStream_SDLRWops::~GpFileStream_SDLRWops()
{
	m_rw->close(m_rw);
}

size_t GpFileStream_SDLRWops::Read(void *bytesOut, size_t size)
{
	return m_rw->read(m_rw, bytesOut, 1, size);
}

size_t GpFileStream_SDLRWops::Write(const void *bytes, size_t size)
{
	return m_rw->write(m_rw, bytes, 1, size);
}

bool GpFileStream_SDLRWops::IsSeekable() const
{
	return true;
}

bool GpFileStream_SDLRWops::IsReadOnly() const
{
	return m_isReadOnly;
}

bool GpFileStream_SDLRWops::IsWriteOnly() const
{
	return m_isWriteOnly;
}

bool GpFileStream_SDLRWops::SeekStart(GpUFilePos_t loc)
{
	return m_rw->seek(m_rw, static_cast<Sint64>(loc), RW_SEEK_SET) >= 0;
}

bool GpFileStream_SDLRWops::SeekCurrent(GpFilePos_t loc)
{
	return m_rw->seek(m_rw, static_cast<Sint64>(loc), RW_SEEK_CUR) >= 0;
}

bool GpFileStream_SDLRWops::SeekEnd(GpUFilePos_t loc)
{
	return m_rw->seek(m_rw, -static_cast<Sint64>(loc), RW_SEEK_END) >= 0;
}

bool GpFileStream_SDLRWops::Truncate(GpUFilePos_t loc)
{
	return false;
}

GpUFilePos_t GpFileStream_SDLRWops::Size() const
{
	return m_rw->size(m_rw);
}

GpUFilePos_t GpFileStream_SDLRWops::GpFileStream_SDLRWops::Tell() const
{
	return SDL_RWtell(m_rw);
}

void GpFileStream_SDLRWops::Close()
{
	this->~GpFileStream_SDLRWops();
	free(this);
}

void GpFileStream_SDLRWops::Flush()
{
}

class GpFileStream_Android_File final : public GpIOStream
{
public:
	GpFileStream_Android_File(FILE *f, int fd, bool readOnly, bool writeOnly);
	~GpFileStream_Android_File();

	size_t Read(void *bytesOut, size_t size) override;
	size_t Write(const void *bytes, size_t size) override;
	bool IsSeekable() const override;
	bool IsReadOnly() const override;
	bool IsWriteOnly() const override;
	bool SeekStart(GpUFilePos_t loc) override;
	bool SeekCurrent(GpFilePos_t loc) override;
	bool SeekEnd(GpUFilePos_t loc) override;
	bool Truncate(GpUFilePos_t loc) override;
	GpUFilePos_t Size() const override;
	GpUFilePos_t Tell() const override;
	void Close() override;
	void Flush() override;

private:
	FILE *m_f;
	int m_fd;
	bool m_seekable;
	bool m_isReadOnly;
	bool m_isWriteOnly;
};

GpFileStream_Android_File::GpFileStream_Android_File(FILE *f, int fd, bool readOnly, bool writeOnly)
	: m_f(f)
	, m_fd(fd)
	, m_isReadOnly(readOnly)
	, m_isWriteOnly(writeOnly)
{
	m_seekable = (fseek(m_f, 0, SEEK_CUR) == 0);
}

GpFileStream_Android_File::~GpFileStream_Android_File()
{
	fclose(m_f);
}

size_t GpFileStream_Android_File::Read(void *bytesOut, size_t size)
{
	if (m_isWriteOnly)
		return 0;
	return fread(bytesOut, 1, size, m_f);
}

size_t GpFileStream_Android_File::Write(const void *bytes, size_t size)
{
	if (m_isReadOnly)
		return 0;
	return fwrite(bytes, 1, size, m_f);
}

bool GpFileStream_Android_File::IsSeekable() const
{
	return m_seekable;
}

bool GpFileStream_Android_File::IsReadOnly() const
{
	return m_isReadOnly;
}

bool GpFileStream_Android_File::IsWriteOnly() const
{
	return m_isWriteOnly;
}

bool GpFileStream_Android_File::SeekStart(GpUFilePos_t loc)
{
	if (!m_seekable)
		return false;

	fflush(m_f);
	return lseek64(m_fd, static_cast<off64_t>(loc), SEEK_SET) >= 0;
}

bool GpFileStream_Android_File::SeekCurrent(GpFilePos_t loc)
{
	if (!m_seekable)
		return false;

	fflush(m_f);
	return lseek64(m_fd, static_cast<off64_t>(loc), SEEK_CUR) >= 0;
}

bool GpFileStream_Android_File::SeekEnd(GpUFilePos_t loc)
{
	if (!m_seekable)
		return false;

	fflush(m_f);
	return lseek64(m_fd, -static_cast<off64_t>(loc), SEEK_END) >= 0;
}

bool GpFileStream_Android_File::Truncate(GpUFilePos_t loc)
{
	fflush(m_f);
	return ftruncate64(m_fd, static_cast<off64_t>(loc)) >= 0;
}

GpUFilePos_t GpFileStream_Android_File::Size() const
{
	fflush(m_f);

	struct stat64 s;
	if (fstat64(m_fd, &s) < 0)
		return 0;

	return static_cast<GpUFilePos_t>(s.st_size);
}

GpUFilePos_t GpFileStream_Android_File::Tell() const
{
	return static_cast<GpUFilePos_t>(ftell(m_f));
}

void GpFileStream_Android_File::Close()
{
	this->~GpFileStream_Android_File();
	free(this);
}

void GpFileStream_Android_File::Flush()
{
	fflush(m_f);
}

bool GpFileSystem_Android::ResolvePathInDownloadsDirectory(PortabilityLayer::VirtualDirectory_t virtualDirectory, char const* const* paths, size_t numPaths, std::string &resolution, bool &isAsset)
{
	JNIEnv *jni = static_cast<JNIEnv *>(SDL_AndroidGetJNIEnv());

	jstring fname = jni->NewStringUTF(paths[0]);

	jobject resultName = jni->CallObjectMethod(m_activity, this->m_selectSourceExportPathMID, fname);
	int n = 0;
	jstring resultPath = static_cast<jstring>(resultName);
	jni->DeleteLocalRef(fname);

	const char *pathStrChars = jni->GetStringUTFChars(resultPath, nullptr);
	resolution = std::string(pathStrChars, static_cast<size_t>(jni->GetStringUTFLength(resultPath)));

	jni->ReleaseStringUTFChars(resultPath, pathStrChars);
	jni->DeleteLocalRef(resultPath);

	resolution = SDL_AndroidGetExternalStoragePath();
	resolution += "/SourceCode.zip";

	isAsset = false;

	return true;
}

bool GpFileSystem_Android::ResolvePath(PortabilityLayer::VirtualDirectory_t virtualDirectory, char const* const* paths, size_t numPaths, std::string &resolution, bool &isAsset)
{
	const char *prefsAppend = nullptr;

	isAsset = false;
	switch (virtualDirectory)
	{
	case PortabilityLayer::VirtualDirectories::kApplicationData:
		resolution = std::string("Packaged") ;
		isAsset = true;
		break;
	case PortabilityLayer::VirtualDirectories::kGameData:
		resolution = std::string("Packaged/Houses");
		isAsset = true;
		break;
	case PortabilityLayer::VirtualDirectories::kFonts:
		resolution = std::string("Resources");
		isAsset = true;
		break;
	case PortabilityLayer::VirtualDirectories::kHighScores:
		prefsAppend = "HighScores";
		break;
	case PortabilityLayer::VirtualDirectories::kUserData:
		prefsAppend = "Houses";
		break;
	case PortabilityLayer::VirtualDirectories::kUserSaves:
		prefsAppend = "SavedGames";
		break;
	case PortabilityLayer::VirtualDirectories::kPrefs:
		prefsAppend = "Prefs";
		break;
	case PortabilityLayer::VirtualDirectories::kSourceExport:
		return ResolvePathInDownloadsDirectory(virtualDirectory, paths, numPaths, resolution, isAsset);
	default:
		return false;
	};

	if (prefsAppend)
	{
		char *prefsDir = SDL_GetPrefPath("aerofoil", "aerofoil");
		resolution = prefsDir;
		SDL_free(prefsDir);
		resolution += prefsAppend;
	}

	for (size_t i = 0; i < numPaths; i++)
	{
		resolution += "/";
		resolution += paths[i];
	}

	return true;
}

GpFileSystem_Android::GpFileSystem_Android()
	: m_activity(nullptr)
	, m_relay(nullptr)
{
}

GpFileSystem_Android::~GpFileSystem_Android()
{
}

void GpFileSystem_Android::InitJNI()
{
	JNIEnv *jni = static_cast<JNIEnv *>(SDL_AndroidGetJNIEnv());

	jobject activityLR = static_cast<jobject>(SDL_AndroidGetActivity());
	jclass activityClassLR = static_cast<jclass>(jni->GetObjectClass(activityLR));

	m_scanAssetDirectoryMID = jni->GetMethodID(activityClassLR, "scanAssetDirectory", "(Ljava/lang/String;)[Ljava/lang/String;");
	m_selectSourceExportPathMID = jni->GetMethodID(activityClassLR, "selectSourceExportPath", "(Ljava/lang/String;)Ljava/lang/String;");

	m_activity = jni->NewGlobalRef(activityLR);

	jni->DeleteLocalRef(activityLR);
	jni->DeleteLocalRef(activityClassLR);

	char *prefsDir = SDL_GetPrefPath("aerofoil", "aerofoil");
	size_t prefsDirLen = strlen(prefsDir);
	for (size_t i = 0; i < prefsDirLen; i++)
	{
		if (prefsDir[i] == '/')
		{
			prefsDir[i] = '\0';
			int created = mkdir(prefsDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
			prefsDir[i] = '/';
		}
	}
	const char *extensions[] = { "HighScores", "Houses", "SavedGames", "Prefs" };
	for (size_t i = 0; i < sizeof(extensions) / sizeof(extensions[0]); i++)
	{
		std::string prefsPath = std::string(prefsDir) + extensions[i];
		int created = mkdir(prefsPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		int n = 0;
	}
	SDL_free(prefsDir);
}

void GpFileSystem_Android::ShutdownJNI()
{
	JNIEnv *jni = static_cast<JNIEnv *>(SDL_AndroidGetJNIEnv());

	jni->DeleteGlobalRef(m_activity);
	m_activity = nullptr;
}

bool GpFileSystem_Android::FileExists(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *path)
{
	std::string resolvedPath;
	bool isAsset;
	if (!ResolvePath(virtualDirectory, &path, 1, resolvedPath, isAsset))
		return false;

	if (isAsset)
	{
		SDL_RWops *rw = SDL_RWFromFile(resolvedPath.c_str(), "rb");
		if (!rw)
			return false;
		SDL_RWclose(rw);
		return true;
	}

	struct stat s;
	return stat(resolvedPath.c_str(), &s) == 0;
}

bool GpFileSystem_Android::FileLocked(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *path, bool *exists)
{
	std::string resolvedPath;
	bool isAsset;
	if (!ResolvePath(virtualDirectory, &path, 1, resolvedPath, isAsset))
	{
		if (exists)
			*exists = false;
		return false;
	}

	if (isAsset)
	{
		if (exists)
			*exists = this->FileExists(virtualDirectory, path);
		return true;
	}

	int permissions = access(resolvedPath.c_str(), W_OK | F_OK);
	*exists = ((permissions & F_OK) != 0);
	return ((permissions & W_OK) != 0);
}

GpIOStream *GpFileSystem_Android::OpenFileNested(PortabilityLayer::VirtualDirectory_t virtualDirectory, char const* const* subPaths, size_t numSubPaths, bool writeAccess, GpFileCreationDisposition_t createDisposition)
{
	const char *mode = nullptr;
	bool canWrite = false;
	bool needResetPosition = false;

	switch (createDisposition)
	{
		case GpFileCreationDispositions::kCreateOrOverwrite:
			mode = "w+b";
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
	bool isAsset;
	if (!ResolvePath(virtualDirectory, subPaths, numSubPaths, resolvedPath, isAsset))
		return nullptr;

	if (isAsset)
	{
		if (createDisposition == GpFileCreationDispositions::kOverwriteExisting || writeAccess)
			return nullptr;

		void *objStorage = malloc(sizeof(GpFileStream_SDLRWops));
		if (!objStorage)
			return nullptr;

		SDL_RWops *rw = SDL_RWFromFile(resolvedPath.c_str(), mode);
		if (!rw)
		{
			free(objStorage);
			return nullptr;
		}

		return new (objStorage) GpFileStream_SDLRWops(rw, true, false);
	}
	else
	{
		void *objStorage = malloc(sizeof(GpFileStream_Android_File));
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

		int fd = fileno(f);

		if (createDisposition == GpFileCreationDispositions::kOverwriteExisting)
		{
			if (ftruncate64(fd, 0) < 0)
			{
				free(objStorage);
				fclose(f);
				return nullptr;
			}
		}

		return new (objStorage) GpFileStream_Android_File(f, fd, !writeAccess, false);
	}
}

bool GpFileSystem_Android::DeleteFile(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *path, bool &existed)
{
	std::string resolvedPath;
	bool isAsset;
	if (!ResolvePath(virtualDirectory, &path, 1, resolvedPath, isAsset))
	{
		existed = false;
		return false;
	}

	if (isAsset)
		return false;

	if (unlink(resolvedPath.c_str()) < 0)
	{
		existed = (errno != ENOENT);
		return false;
	}
	existed = true;
	return true;
}

PortabilityLayer::HostDirectoryCursor *GpFileSystem_Android::ScanDirectoryNested(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *const *paths, size_t numPaths)
{
	ScanDirectoryNestedContext ctx;
	ctx.m_this = this;
	ctx.m_returnValue = nullptr;
	ctx.m_virtualDirectory = virtualDirectory;
	ctx.m_paths = paths;
	ctx.m_numPaths = numPaths;
	m_relay->Invoke(ScanDirectoryNestedThunk, &ctx);

	return ctx.m_returnValue;
}

void GpFileSystem_Android::ScanDirectoryNestedThunk(void *context)
{
	ScanDirectoryNestedContext *ctx = static_cast<ScanDirectoryNestedContext*>(context);
	ctx->m_returnValue = ctx->m_this->ScanDirectoryNestedInternal(ctx->m_virtualDirectory, ctx->m_paths, ctx->m_numPaths);
}

PortabilityLayer::HostDirectoryCursor *GpFileSystem_Android::ScanDirectoryNestedInternal(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *const *paths, size_t numPaths)
{
	if (virtualDirectory == PortabilityLayer::VirtualDirectories::kGameData || virtualDirectory == PortabilityLayer::VirtualDirectories::kApplicationData)
		return ScanAssetDirectory(virtualDirectory, paths, numPaths);

	return ScanStorageDirectory(virtualDirectory, paths, numPaths);
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

bool GpFileSystem_Android::IsVirtualDirectoryLooseResources(PortabilityLayer::VirtualDirectory_t virtualDir) const
{
	return false;
}

void GpFileSystem_Android::SetMainThreadRelay(IGpThreadRelay *relay)
{
	m_relay = relay;
}

GpFileSystem_Android *GpFileSystem_Android::GetInstance()
{
	return &ms_instance;
}

class GpDirectoryCursor_StringList final : public PortabilityLayer::HostDirectoryCursor
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

class GpDirectoryCursor_POSIX final : public PortabilityLayer::HostDirectoryCursor
{
public:
	explicit GpDirectoryCursor_POSIX(DIR *dir);
	~GpDirectoryCursor_POSIX();

	bool GetNext(const char *&outFileName) override;
	void Destroy() override;

private:
	DIR *m_dir;
};

GpDirectoryCursor_POSIX::GpDirectoryCursor_POSIX(DIR *dir)
	: m_dir(dir)
{
}

GpDirectoryCursor_POSIX::~GpDirectoryCursor_POSIX()
{
	closedir(m_dir);
}

bool GpDirectoryCursor_POSIX::GetNext(const char *&outFileName)
{
	struct dirent *dir = readdir(m_dir);
	if (!dir)
		return false;

	outFileName = dir->d_name;
	return true;
}

void GpDirectoryCursor_POSIX::Destroy()
{
	delete this;
}

PortabilityLayer::HostDirectoryCursor *GpFileSystem_Android::ScanAssetDirectory(PortabilityLayer::VirtualDirectory_t virtualDirectory, char const* const* paths, size_t numPaths)
{

	std::string resolvedPath;
	std::vector<std::string> subPaths;
	bool isAsset = true;
	if (!ResolvePath(virtualDirectory, paths, numPaths, resolvedPath, isAsset))
		return nullptr;

	JNIEnv *jni = static_cast<JNIEnv *>(SDL_AndroidGetJNIEnv());

	jstring directory = jni->NewStringUTF(resolvedPath.c_str());

	jobjectArray resultArray = static_cast<jobjectArray>(jni->CallObjectMethod(m_activity, m_scanAssetDirectoryMID, directory));
	jni->DeleteLocalRef(directory);

	size_t arrayLength = jni->GetArrayLength(resultArray);
	subPaths.reserve(arrayLength);
	for (size_t i = 0; i < arrayLength; i++)
	{
		jstring pathJStr = static_cast<jstring>(jni->GetObjectArrayElement(resultArray, i));
		const char *pathStrChars = jni->GetStringUTFChars(pathJStr, nullptr);

		subPaths.push_back(std::string(pathStrChars, static_cast<size_t>(jni->GetStringUTFLength(pathJStr))));

		jni->ReleaseStringUTFChars(pathJStr, pathStrChars);
		jni->DeleteLocalRef(pathJStr);
	}

	jni->DeleteLocalRef(resultArray);

	return new GpDirectoryCursor_StringList(subPaths);
}

PortabilityLayer::HostDirectoryCursor *GpFileSystem_Android::ScanStorageDirectory(PortabilityLayer::VirtualDirectory_t virtualDirectory, char const* const* paths, size_t numPaths)
{
	std::string resolvedPath;
	std::vector<std::string> subPaths;
	bool isAsset = true;
	if (!ResolvePath(virtualDirectory, paths, numPaths, resolvedPath, isAsset))
		return nullptr;

	DIR *d = opendir(resolvedPath.c_str());
	if (!d)
		return nullptr;

	return new GpDirectoryCursor_POSIX(d);
}

GpFileSystem_Android GpFileSystem_Android::ms_instance;
