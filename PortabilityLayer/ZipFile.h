#pragma once

#include "PLLittleEndian.h"

#include <stdint.h>

namespace PortabilityLayer
{
	namespace ZipConstants
	{
		static const uint16_t kStoredRequiredVersion = 10;
		static const uint16_t kDirectoryRequiredVersion = 20;
		static const uint16_t kCompressedRequiredVersion = 20;

		static const uint32_t kDirectoryAttributes = 0x0010;
		static const uint32_t kArchivedAttributes = 0x0020;

		static const uint16_t kStoredMethod = 0x0000;
		static const uint16_t kDeflatedMethod = 0x0008;
	}

	struct ZipFileLocalHeader
	{
		static const uint32_t kSignature = 0x04034b50;

		LEUInt32_t m_signature;
		LEUInt16_t m_versionRequired;
		LEUInt16_t m_flags;
		LEUInt16_t m_method;
		LEUInt16_t m_modificationTime;
		LEUInt16_t m_modificationDate;
		LEUInt32_t m_crc;
		LEUInt32_t m_compressedSize;
		LEUInt32_t m_uncompressedSize;
		LEUInt16_t m_fileNameLength;
		LEUInt16_t m_extraFieldLength;
	};

	struct ZipCentralDirectoryFileHeader
	{
		static const uint32_t kSignature = 0x02014b50;

		LEUInt32_t m_signature;
		LEUInt16_t m_versionCreated;
		LEUInt16_t m_versionRequired;
		LEUInt16_t m_flags;
		LEUInt16_t m_method;
		LEUInt16_t m_modificationTime;
		LEUInt16_t m_modificationDate;
		LEUInt32_t m_crc;
		LEUInt32_t m_compressedSize;
		LEUInt32_t m_uncompressedSize;
		LEUInt16_t m_fileNameLength;
		LEUInt16_t m_extraFieldLength;
		LEUInt16_t m_commentLength;
		LEUInt16_t m_diskNumber;
		LEUInt16_t m_internalAttributes;
		LEUInt32_t m_externalAttributes;
		LEUInt32_t m_localHeaderOffset;

		// File name
		// Extra field
		// File comment
	};

	struct ZipEndOfCentralDirectoryRecord
	{
		static const uint32_t kSignature = 0x06054b50;

		LEUInt32_t m_signature;
		LEUInt16_t m_thisDiskNumber;
		LEUInt16_t m_centralDirDisk;
		LEUInt16_t m_numCentralDirRecordsThisDisk;
		LEUInt16_t m_numCentralDirRecords;
		LEUInt32_t m_centralDirectorySizeBytes;
		LEUInt32_t m_centralDirStartOffset;
		LEUInt16_t m_commentLength;

		// Comment
	};
}
