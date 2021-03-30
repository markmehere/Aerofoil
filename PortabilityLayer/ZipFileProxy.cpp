#include "ZipFileProxy.h"

#include "BinarySearch.h"
#include "FileSectionStream.h"
#include "GpIOStream.h"
#include "InflateStream.h"
#include "MemoryManager.h"
#include "ZipFile.h"

#include "DeflateCodec.h"

#include <algorithm>

namespace
{
	static const char *GetZipItemName(const PortabilityLayer::UnalignedPtr<PortabilityLayer::ZipCentralDirectoryFileHeader> &itemPtr)
	{
		return reinterpret_cast<const char*>(itemPtr.GetRawPtr()) + sizeof(PortabilityLayer::ZipCentralDirectoryFileHeader);
	}

	static int ZipDirectorySearchPredicateResolved(const char *path, const char *itemPath, uint16_t itemPathLength)
	{
		for (size_t i = 0; i < itemPathLength; i++)
		{
			const uint8_t pathC = static_cast<uint8_t>(path[i] & 0xff);

			if (pathC == 0)
				return -1;

			const uint8_t itemC = static_cast<uint8_t>(itemPath[i] & 0xff);

			if (pathC < itemC)
				return -1;
			if (pathC > itemC)
				return 1;
		}

		if (path[itemPathLength] == 0)
			return 0;
		else
			return 1;
	}

	static int ZipDirectorySearchPredicate(const char *path, const PortabilityLayer::UnalignedPtr<PortabilityLayer::ZipCentralDirectoryFileHeader> &itemPtr)
	{
		const uint16_t fnameLength = itemPtr.Get().m_fileNameLength;
		const char *itemPath = GetZipItemName(itemPtr);

		return ZipDirectorySearchPredicateResolved(path, itemPath, fnameLength);
	}

	static int ZipDirectorySortPredicate(const void *a, const void *b)
	{
		const PortabilityLayer::ZipCentralDirectoryFileHeader *typedA = *static_cast<PortabilityLayer::ZipCentralDirectoryFileHeader *const*>(a);
		const PortabilityLayer::ZipCentralDirectoryFileHeader *typedB = *static_cast<PortabilityLayer::ZipCentralDirectoryFileHeader *const*>(b);

		const uint16_t lenA = typedA->m_fileNameLength;
		const uint16_t lenB = typedB->m_fileNameLength;

		const char *pathA = reinterpret_cast<const char*>(typedA) + sizeof(PortabilityLayer::ZipCentralDirectoryFileHeader);
		const char *pathB = reinterpret_cast<const char*>(typedB) + sizeof(PortabilityLayer::ZipCentralDirectoryFileHeader);

		const size_t shorterLength = (lenA < lenB) ? lenA : lenB;

		for (size_t i = 0; i < shorterLength; i++)
		{
			const uint8_t ca = static_cast<uint8_t>(pathA[i] & 0xff);
			const uint8_t cb = static_cast<uint8_t>(pathB[i] & 0xff);

			if (ca < cb)
				return -1;
			if (ca > cb)
				return 1;
		}

		if (lenA < lenB)
			return -1;
		if (lenA > lenB)
			return 1;

		return 0;
	}

	static bool CheckAndFixFileName(uint8_t *fname, size_t length)
	{
		for (size_t i = 0; i < length; i++)
		{
			if (fname[i] == '\\')
				fname[i] = '/';
			else if (fname[i] == 0)
				return false;	// Illegal null in file path, illegal
		}

		return true;
	}
}

namespace PortabilityLayer
{
	void ZipFileProxy::Destroy()
	{
		MemoryManager *mm = MemoryManager::GetInstance();

		this->~ZipFileProxy();
		mm->Release(this);
	}

	bool ZipFileProxy::IndexFile(const char *path, size_t &outIndex) const
	{
		UnalignedPtr<ZipCentralDirectoryFileHeader> *indexLoc = BinarySearch(m_sortedFiles, m_sortedFiles + m_numFiles, path, ZipDirectorySearchPredicate);
		const size_t index = static_cast<size_t>(indexLoc - m_sortedFiles);

		if (index == m_numFiles)
			return false;
		else
		{
			outIndex = index;
			return true;
		}
	}


	bool ZipFileProxy::HasPrefix(const char *prefix) const
	{
		size_t prefixLen = strlen(prefix);

		size_t firstFile = 0;
		size_t lastFileExclusive = m_numFiles;
		size_t midFile = (firstFile + lastFileExclusive) / 2;

		while (firstFile != lastFileExclusive)
		{
			const UnalignedPtr<PortabilityLayer::ZipCentralDirectoryFileHeader> itemPtr = m_sortedFiles[midFile];
			const PortabilityLayer::ZipCentralDirectoryFileHeader item = itemPtr.Get();
			const uint16_t itemNameLength = item.m_fileNameLength;
			const char *itemPath = GetZipItemName(itemPtr);

			const int delta = ZipDirectorySearchPredicateResolved(prefix, itemPath, itemNameLength);

			// -1 = path precedes item, 1 = path succeeds item
			if (delta < 0)
			{
				const bool isPathPrefix = ((itemNameLength > prefixLen) && !memcmp(prefix, itemPath, prefixLen));
				if (isPathPrefix)
					return true;
				else
				{
					lastFileExclusive = midFile;
					midFile = (firstFile + lastFileExclusive) / 2;
				}
			}
			else if (delta > 0)
			{
				firstFile = midFile + 1;
				midFile = (firstFile + lastFileExclusive) / 2;
			}
			else //if(delta == 0)
			{
				// Found the directory
				firstFile = midFile + 1;
				midFile = (firstFile + lastFileExclusive) / 2;
			}
		}

		return false;
	}

	bool ZipFileProxy::FindFirstWithPrefix(const char *prefix, size_t &outFileIndex) const
	{
		size_t prefixLen = strlen(prefix);

		// Could do binary search, but it's much more complicated in this case, just do linear scan...
		for (size_t i = 0; i < m_numFiles; i++)
		{
			const UnalignedPtr<PortabilityLayer::ZipCentralDirectoryFileHeader> itemPtr = m_sortedFiles[i];
			const PortabilityLayer::ZipCentralDirectoryFileHeader item = itemPtr.Get();

			const uint16_t itemNameLength = item.m_fileNameLength;
			const char *itemPath = GetZipItemName(itemPtr);

			const int delta = ZipDirectorySearchPredicateResolved(prefix, itemPath, itemNameLength);
			if (delta == 0)
				continue;

			if (delta < 0)
			{
				const bool isPathPrefix = ((itemNameLength > prefixLen) && !memcmp(prefix, itemPath, prefixLen));

				if (isPathPrefix)
				{
					outFileIndex = i;
					return true;
				}
				else
					return false;
			}
		}

		return false;
	}

	bool ZipFileProxy::LoadFile(size_t index, void *outBuffer)
	{
		ZipCentralDirectoryFileHeader centralDirHeader = m_sortedFiles[index].Get();

		if (!m_stream->SeekStart(centralDirHeader.m_localHeaderOffset))
			return false;

		ZipFileLocalHeader localHeader;
		if (m_stream->Read(&localHeader, sizeof(ZipFileLocalHeader)) != sizeof(ZipFileLocalHeader))
			return false;

		if (!m_stream->SeekCurrent(localHeader.m_fileNameLength + localHeader.m_extraFieldLength))
			return false;

		if (localHeader.m_compressedSize != centralDirHeader.m_compressedSize || localHeader.m_uncompressedSize != centralDirHeader.m_uncompressedSize || localHeader.m_method != centralDirHeader.m_method)
			return false;

		const size_t uncompressedSize = centralDirHeader.m_uncompressedSize;
		if (localHeader.m_method == PortabilityLayer::ZipConstants::kStoredMethod)
			return m_stream->Read(outBuffer, uncompressedSize) == uncompressedSize;
		else if (localHeader.m_method == PortabilityLayer::ZipConstants::kDeflatedMethod)
		{
			const size_t compressedSize = centralDirHeader.m_compressedSize;

			return DeflateCodec::DecompressStream(m_stream, compressedSize, outBuffer, uncompressedSize);
		}
		else
			return false;
	}

	GpIOStream *ZipFileProxy::OpenFile(size_t index) const
	{
		ZipCentralDirectoryFileHeader centralDirHeader = m_sortedFiles[index].Get();

		if (!m_stream->SeekStart(centralDirHeader.m_localHeaderOffset))
			return nullptr;

		ZipFileLocalHeader localHeader;
		if (m_stream->Read(&localHeader, sizeof(ZipFileLocalHeader)) != sizeof(ZipFileLocalHeader))
			return nullptr;

		if (!m_stream->SeekCurrent(localHeader.m_fileNameLength + localHeader.m_extraFieldLength))
			return nullptr;

		if (localHeader.m_compressedSize != centralDirHeader.m_compressedSize || localHeader.m_uncompressedSize != centralDirHeader.m_uncompressedSize || localHeader.m_method != centralDirHeader.m_method)
			return nullptr;

		const size_t compressedSize = centralDirHeader.m_compressedSize;
		const size_t uncompressedSize = centralDirHeader.m_uncompressedSize;
		if (localHeader.m_method == PortabilityLayer::ZipConstants::kStoredMethod)
		{
			if (uncompressedSize != compressedSize)
				return nullptr;

			return FileSectionStream::Create(m_stream, m_stream->Tell(), uncompressedSize);
		}
		else if (localHeader.m_method == PortabilityLayer::ZipConstants::kDeflatedMethod)
			return InflateStream::Create(m_stream, m_stream->Tell(), compressedSize, uncompressedSize);
		else
			return nullptr;
	}

	size_t ZipFileProxy::NumFiles() const
	{
		return m_numFiles;
	}

	size_t ZipFileProxy::GetFileSize(size_t index) const
	{
		return m_sortedFiles[index].Get().m_uncompressedSize;
	}

	void ZipFileProxy::GetFileName(size_t index, const char *&outName, size_t &outLength) const
	{
		const UnalignedPtr<PortabilityLayer::ZipCentralDirectoryFileHeader> itemPtr = m_sortedFiles[index];
		const PortabilityLayer::ZipCentralDirectoryFileHeader item = itemPtr.Get();

		outLength = item.m_fileNameLength;
		outName = GetZipItemName(itemPtr);
	}

	ZipFileProxy *ZipFileProxy::Create(GpIOStream *stream)
	{
		MemoryManager *mm = MemoryManager::GetInstance();

		if (!stream->SeekEnd(sizeof(ZipEndOfCentralDirectoryRecord)))
		{
			fprintf(stderr, "Seek end failed\n");
			return nullptr;
		}

		ZipEndOfCentralDirectoryRecord eocd;
		if (stream->Read(&eocd, sizeof(eocd)) != sizeof(eocd))
		{
			fprintf(stderr, "EOCD read failed\n");
			return nullptr;
		}

		if (eocd.m_signature != ZipEndOfCentralDirectoryRecord::kSignature)
		{
			fprintf(stderr, "EOCD sig check failed\n");
			return nullptr;
		}

		if (!stream->SeekStart(eocd.m_centralDirStartOffset))
		{
			fprintf(stderr, "CDir seek failed\n");
			return nullptr;
		}

		const size_t centralDirSize = eocd.m_centralDirectorySizeBytes;
		void *centralDirImage = nullptr;
		UnalignedPtr<ZipCentralDirectoryFileHeader> *centralDirFiles = nullptr;

		const size_t numFiles = eocd.m_numCentralDirRecords;

		if (centralDirSize > 0)
		{
			centralDirImage = mm->Alloc(centralDirSize);
			if (!centralDirImage)
				return nullptr;

			centralDirFiles = static_cast<UnalignedPtr<ZipCentralDirectoryFileHeader>*>(mm->Alloc(sizeof(UnalignedPtr<ZipCentralDirectoryFileHeader>) * numFiles));
			if (!centralDirFiles)
			{
				mm->Release(centralDirImage);
				return nullptr;
			}

			if (stream->Read(centralDirImage, centralDirSize) != centralDirSize)
			{
				fprintf(stderr, "CDir image read failed\n");

				mm->Release(centralDirFiles);
				mm->Release(centralDirImage);
				return nullptr;
			}
		}

		bool failed = false;

		uint8_t *const centralDirStart = static_cast<uint8_t*>(centralDirImage);
		uint8_t *const centralDirEnd = centralDirStart + centralDirSize;
		uint8_t *centralDirCursor = centralDirStart;

		for (size_t i = 0; i < numFiles; i++)
		{
			if (centralDirEnd - centralDirCursor < sizeof(ZipCentralDirectoryFileHeader))
			{
				fprintf(stderr, "CDir read failed point 1\n");

				failed = true;
				break;
			}

			UnalignedPtr<ZipCentralDirectoryFileHeader> centralDirHeaderPtr = UnalignedPtr<ZipCentralDirectoryFileHeader>(reinterpret_cast<ZipCentralDirectoryFileHeader*>(centralDirCursor));
			ZipCentralDirectoryFileHeader centralDirHeader = centralDirHeaderPtr.Get();

			centralDirCursor += sizeof(ZipCentralDirectoryFileHeader);

			if (centralDirHeader.m_signature != ZipCentralDirectoryFileHeader::kSignature)
			{
				fprintf(stderr, "CDir read failed point 2\n");

				failed = true;
				break;
			}

			if (centralDirEnd - centralDirCursor < centralDirHeader.m_fileNameLength)
			{
				fprintf(stderr, "CDir read failed point 3\n");

				failed = true;
				break;
			}

			if (!CheckAndFixFileName(centralDirCursor, centralDirHeader.m_fileNameLength))
			{
				fprintf(stderr, "CDir read failed point 4\n");

				failed = true;
				break;
			}

			centralDirCursor += centralDirHeader.m_fileNameLength;

			if (centralDirEnd - centralDirCursor < centralDirHeader.m_extraFieldLength)
			{
				fprintf(stderr, "CDir read failed point 5\n");

				failed = true;
				break;
			}

			centralDirCursor += centralDirHeader.m_extraFieldLength;

			if (centralDirEnd - centralDirCursor < centralDirHeader.m_commentLength)
			{
				fprintf(stderr, "CDir read failed point 6\n");

				failed = true;
				break;
			}

			centralDirCursor += centralDirHeader.m_commentLength;

			centralDirFiles[i] = centralDirHeaderPtr;
		}

		if (failed)
		{
			mm->Release(centralDirFiles);
			mm->Release(centralDirImage);
			return nullptr;
		}

		if (numFiles)
			qsort(centralDirFiles, numFiles, sizeof(ZipCentralDirectoryFileHeader*), ZipDirectorySortPredicate);

		for (size_t i = 1; i < numFiles; i++)
		{
			if (ZipDirectorySortPredicate(centralDirFiles + (i - 1), centralDirFiles + i) == 0)
			{
				fprintf(stderr, "File names were duplicated\n");

				// Duplicate file names
				mm->Release(centralDirFiles);
				mm->Release(centralDirImage);
				return nullptr;
			}
		}

		void *storage = mm->Alloc(sizeof(ZipFileProxy));
		if (!storage)
		{
			mm->Release(centralDirFiles);
			mm->Release(centralDirImage);
			return nullptr;
		}

		return new (storage) ZipFileProxy(stream, centralDirImage, centralDirFiles, numFiles);
	}

	ZipFileProxy::ZipFileProxy(GpIOStream *stream, void *centralDirImage, UnalignedPtr<ZipCentralDirectoryFileHeader> *sortedFiles, size_t numFiles)
		: m_stream(stream)
		, m_centralDirImage(centralDirImage)
		, m_sortedFiles(sortedFiles)
		, m_numFiles(numFiles)
	{
	}

	ZipFileProxy::~ZipFileProxy()
	{
		MemoryManager *mm = MemoryManager::GetInstance();
		mm->Release(m_centralDirImage);
		mm->Release(m_sortedFiles);
	}
}
