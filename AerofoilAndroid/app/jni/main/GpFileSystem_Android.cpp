#define _LARGEFILE64_SOURCE
#include "GpFileSystem_Android.h"
#include "GpIOStream.h"
#include "VirtualDirectory.h"

#include "SDL_rwops.h"

#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>



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

	return lseek64(m_fd, static_cast<off64_t>(loc), SEEK_SET) >= 0;
}

bool GpFileStream_Android_File::SeekCurrent(GpFilePos_t loc)
{
	if (!m_seekable)
		return false;

	return lseek64(m_fd, static_cast<off64_t>(loc), SEEK_CUR) >= 0;
}

bool GpFileStream_Android_File::SeekEnd(GpUFilePos_t loc)
{
	if (!m_seekable)
		return false;

	return lseek64(m_fd, -static_cast<off64_t>(loc), SEEK_END) >= 0;
}

bool GpFileStream_Android_File::Truncate(GpUFilePos_t loc)
{
	return ftruncate64(m_fd, static_cast<off64_t>(loc)) >= 0;
}

GpUFilePos_t GpFileStream_Android_File::Size() const
{
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

static bool ResolvePath(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *path, std::string &resolution, bool &isAsset)
{
	isAsset = false;
	switch (virtualDirectory)
	{
	case PortabilityLayer::VirtualDirectories::kApplicationData:
		resolution = std::string("Packaged/") + path;
		isAsset = true;
		return true;
	case PortabilityLayer::VirtualDirectories::kGameData:
		resolution = std::string("Packaged/Houses/") + path;
		isAsset = true;
		return true;
	case PortabilityLayer::VirtualDirectories::kFonts:
		resolution = std::string("Resources/") + path;
		isAsset = true;
		return true;
	default:
		return false;
	};
}

GpFileSystem_Android::GpFileSystem_Android()
{
}

bool GpFileSystem_Android::FileExists(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *path)
{
	std::string resolvedPath;
	bool isAsset;
	if (!ResolvePath(virtualDirectory, path, resolvedPath, isAsset))
		return false;

	if (isAsset)
	{
		SDL_RWops *rw = SDL_RWFromFile(resolvedPath.c_str(), "rb");
		if (!rw)
			return false;
		SDL_RWclose(rw);
	}

	struct stat s;
	return stat(resolvedPath.c_str(), &s) == 0;
}

bool GpFileSystem_Android::FileLocked(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *path, bool *exists)
{
	std::string resolvedPath;
	bool isAsset;
	if (!ResolvePath(virtualDirectory, path, resolvedPath, isAsset))
	{
		*exists = false;
		return false;
	}

	if (isAsset)
		return true;

	int permissions = access(resolvedPath.c_str(), W_OK | F_OK);
	*exists = ((permissions & F_OK) != 0);
	return ((permissions & W_OK) != 0);
}

GpIOStream *GpFileSystem_Android::OpenFile(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *path, bool writeAccess, GpFileCreationDisposition_t createDisposition)
{
	const char *mode = nullptr;
	bool canWrite = false;

	switch (createDisposition) {
		case GpFileCreationDispositions::kCreateOrOverwrite:
			mode = "w+b";
			break;
		case GpFileCreationDispositions::kCreateNew:
			mode = "x+b";
			break;
		case GpFileCreationDispositions::kCreateOrOpen:
			mode = "c+b";
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
	if (!ResolvePath(virtualDirectory, path, resolvedPath, isAsset))
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
	if (!ResolvePath(virtualDirectory, path, resolvedPath, isAsset))
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
