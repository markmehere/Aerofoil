#pragma once

#include "PLUnalignedPtr.h"

class GpIOStream;

namespace PortabilityLayer
{
	struct ZipCentralDirectoryFileHeader;

	class ZipFileProxy
	{
	public:
		void Destroy();

		bool IndexFile(const char *path, size_t &outIndex) const;
		bool LoadFile(size_t index, void *outBuffer);

		bool HasPrefix(const char *path) const;
		bool FindFirstWithPrefix(const char *resPrefix, size_t &outFileIndex) const;

		size_t NumFiles() const;
		size_t GetFileSize(size_t index) const;
		void GetFileName(size_t index, const char *&outName, size_t &outLength) const;


		static ZipFileProxy *Create(GpIOStream *stream);

	private:
		ZipFileProxy(GpIOStream *stream, void *centralDirImage, UnalignedPtr<ZipCentralDirectoryFileHeader> *sortedFiles, size_t numFiles);
		~ZipFileProxy();

		GpIOStream *m_stream;
		void *m_centralDirImage;
		UnalignedPtr<ZipCentralDirectoryFileHeader> *m_sortedFiles;
		size_t m_numFiles;
	};
}
