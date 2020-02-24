#pragma once

#include "PLUnalignedPtr.h"

namespace PortabilityLayer
{
	class IOStream;
	struct ZipCentralDirectoryFileHeader;

	class ZipFileProxy
	{
	public:
		void Destroy();

		bool IndexFile(const char *path, size_t &outIndex) const;
		bool LoadFile(size_t index, void *outBuffer);

		bool HasPrefix(const char *path) const;

		size_t NumFiles() const;
		size_t GetFileSize(size_t index) const;

		static ZipFileProxy *Create(IOStream *stream);

	private:
		ZipFileProxy(IOStream *stream, void *centralDirImage, UnalignedPtr<ZipCentralDirectoryFileHeader> *sortedFiles, size_t numFiles);
		~ZipFileProxy();

		IOStream *m_stream;
		void *m_centralDirImage;
		UnalignedPtr<ZipCentralDirectoryFileHeader> *m_sortedFiles;
		size_t m_numFiles;
	};
}
