#include "ZipFileProxy.h"

#include "BinarySearch.h"
#include "IOStream.h"
#include "MemoryManager.h"
#include "ZipFile.h"

#include "DeflateCodec.h"

namespace
{
	static int ZipDirectorySearchPredicate(const char *path, PortabilityLayer::ZipCentralDirectoryFileHeader *item)
	{
		const uint16_t fnameLength = item->m_fileNameLength;
		const char *itemPath = reinterpret_cast<const char*>(item) + sizeof(PortabilityLayer::ZipCentralDirectoryFileHeader);

		for (size_t i = 0; i < fnameLength; i++)
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

		if (path[fnameLength] == 0)
			return 0;
		else
			return 1;
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
		PortabilityLayer::ZipCentralDirectoryFileHeader **indexLoc = BinarySearch(m_sortedFiles, m_sortedFiles + m_numFiles, path, ZipDirectorySearchPredicate);
		const size_t index = static_cast<size_t>(indexLoc - m_sortedFiles);

		if (index == m_numFiles)
			return false;
		else
		{
			outIndex = index;
			return true;
		}
	}

	bool ZipFileProxy::LoadFile(size_t index, void *outBuffer)
	{
		ZipCentralDirectoryFileHeader *centralDirHeader = m_sortedFiles[index];

		if (!m_stream->SeekStart(centralDirHeader->m_localHeaderOffset))
			return false;

		ZipFileLocalHeader localHeader;
		if (m_stream->Read(&localHeader, sizeof(ZipFileLocalHeader)) != sizeof(ZipFileLocalHeader))
			return false;

		if (!m_stream->SeekCurrent(localHeader.m_fileNameLength + localHeader.m_extraFieldLength))
			return false;

		if (localHeader.m_compressedSize != centralDirHeader->m_compressedSize || localHeader.m_uncompressedSize != centralDirHeader->m_uncompressedSize || localHeader.m_method != centralDirHeader->m_method)
			return false;

		const size_t uncompressedSize = centralDirHeader->m_uncompressedSize;
		if (localHeader.m_method == PortabilityLayer::ZipConstants::kStoredMethod)
			return m_stream->Read(outBuffer, uncompressedSize) == uncompressedSize;
		else if (localHeader.m_method == PortabilityLayer::ZipConstants::kDeflatedMethod)
		{
			const size_t compressedSize = centralDirHeader->m_compressedSize;

			return DeflateCodec::DecompressStream(m_stream, compressedSize, outBuffer, uncompressedSize);
		}
		else
			return false;
	}

	size_t ZipFileProxy::NumFiles() const
	{
		return m_numFiles;
	}

	size_t ZipFileProxy::GetFileSize(size_t index) const
	{
		return m_sortedFiles[index]->m_uncompressedSize;
	}

	ZipFileProxy *ZipFileProxy::Create(IOStream *stream)
	{
		MemoryManager *mm = MemoryManager::GetInstance();

		if (!stream->SeekEnd(sizeof(ZipEndOfCentralDirectoryRecord)))
			return nullptr;

		ZipEndOfCentralDirectoryRecord eocd;
		if (stream->Read(&eocd, sizeof(eocd)) != sizeof(eocd))
			return nullptr;

		if (eocd.m_signature != ZipEndOfCentralDirectoryRecord::kSignature)
			return nullptr;

		if (!stream->SeekStart(eocd.m_centralDirStartOffset))
			return nullptr;

		const size_t centralDirSize = eocd.m_centralDirectorySizeBytes;
		void *centralDirImage = mm->Alloc(centralDirSize);
		if (!centralDirImage)
			return nullptr;

		const size_t numFiles = eocd.m_numCentralDirRecords;

		ZipCentralDirectoryFileHeader **centralDirFiles = static_cast<ZipCentralDirectoryFileHeader **>(mm->Alloc(sizeof(ZipCentralDirectoryFileHeader*) * numFiles));
		if (!centralDirFiles)
		{
			mm->Release(centralDirImage);
			return nullptr;
		}

		if (stream->Read(centralDirImage, centralDirSize) != centralDirSize)
		{
			mm->Release(centralDirFiles);
			mm->Release(centralDirImage);
			return nullptr;
		}

		bool failed = false;

		uint8_t *const centralDirStart = static_cast<uint8_t*>(centralDirImage);
		uint8_t *const centralDirEnd = centralDirStart + centralDirSize;
		uint8_t *centralDirCursor = centralDirStart;

		for (size_t i = 0; i < numFiles; i++)
		{
			if (centralDirEnd - centralDirCursor < sizeof(ZipCentralDirectoryFileHeader))
			{
				failed = true;
				break;
			}

			ZipCentralDirectoryFileHeader *centralDirHeader = reinterpret_cast<ZipCentralDirectoryFileHeader*>(centralDirCursor);
			centralDirCursor += sizeof(ZipCentralDirectoryFileHeader);

			if (centralDirHeader->m_signature != ZipCentralDirectoryFileHeader::kSignature)
			{
				failed = true;
				break;
			}

			if (centralDirEnd - centralDirCursor < centralDirHeader->m_fileNameLength)
			{
				failed = true;
				break;
			}

			if (!CheckAndFixFileName(centralDirCursor, centralDirHeader->m_fileNameLength))
			{
				failed = true;
				break;
			}

			centralDirCursor += centralDirHeader->m_fileNameLength;

			if (centralDirEnd - centralDirCursor < centralDirHeader->m_extraFieldLength)
			{
				failed = true;
				break;
			}

			centralDirCursor += centralDirHeader->m_extraFieldLength;

			if (centralDirEnd - centralDirCursor < centralDirHeader->m_commentLength)
			{
				failed = true;
				break;
			}

			centralDirCursor += centralDirHeader->m_commentLength;

			centralDirFiles[i] = centralDirHeader;
		}

		if (failed)
		{
			mm->Release(centralDirFiles);
			mm->Release(centralDirImage);
			return nullptr;
		}

		qsort(centralDirFiles, numFiles, sizeof(ZipCentralDirectoryFileHeader*), ZipDirectorySortPredicate);

		for (size_t i = 1; i < numFiles; i++)
		{
			if (ZipDirectorySortPredicate(centralDirFiles + (i - 1), centralDirFiles + i) == 0)
			{
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

	ZipFileProxy::ZipFileProxy(IOStream *stream, void *centralDirImage, ZipCentralDirectoryFileHeader **sortedFiles, size_t numFiles)
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
