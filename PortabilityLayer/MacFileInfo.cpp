#include "MacFileInfo.h"

#include "DeflateCodec.h"
#include "GpIOStream.h"
#include "ZipFile.h"
#include "PLBigEndian.h"
#include "CombinedTimestamp.h"

#include <string.h>

namespace PortabilityLayer
{
	void MacFilePropertiesSerialized::Deserialize(MacFileProperties &props) const
	{
		memcpy(props.m_fileType, m_data + kOffsetFileType, 4);
		memcpy(props.m_fileCreator, m_data + kOffsetFileCreator, 4);
		memcpy(&props.m_xPos, m_data + kOffsetXPos, 2);
		memcpy(&props.m_yPos, m_data + kOffsetYPos, 2);
		memcpy(&props.m_finderFlags, m_data + kOffsetFinderFlags, 2);
		memcpy(&props.m_protected, m_data + kOffsetProtected, 1);
		memcpy(&props.m_createdTimeMacEpoch, m_data + kOffsetCreatedDate, 8);
		memcpy(&props.m_modifiedTimeMacEpoch, m_data + kOffsetModifiedDate, 8);

		PortabilityLayer::ByteSwap::BigInt16(props.m_xPos);
		PortabilityLayer::ByteSwap::BigInt16(props.m_yPos);
		PortabilityLayer::ByteSwap::BigUInt16(props.m_finderFlags);
		PortabilityLayer::ByteSwap::BigInt64(props.m_createdTimeMacEpoch);
		PortabilityLayer::ByteSwap::BigInt64(props.m_modifiedTimeMacEpoch);
	}

	void MacFilePropertiesSerialized::Serialize(const MacFileProperties &props)
	{
		int16_t xPos = props.m_xPos;
		int16_t yPos = props.m_yPos;
		uint16_t finderFlags = props.m_finderFlags;
		uint64_t createdDate = props.m_createdTimeMacEpoch;
		uint64_t modifiedDate = props.m_modifiedTimeMacEpoch;

		PortabilityLayer::ByteSwap::BigInt16(xPos);
		PortabilityLayer::ByteSwap::BigInt16(yPos);
		PortabilityLayer::ByteSwap::BigUInt16(finderFlags);
		PortabilityLayer::ByteSwap::BigUInt64(createdDate);
		PortabilityLayer::ByteSwap::BigUInt64(modifiedDate);

		memcpy(m_data + kOffsetFileType, props.m_fileType, 4);
		memcpy(m_data + kOffsetFileCreator, props.m_fileCreator, 4);
		memcpy(m_data + kOffsetXPos, &xPos, 2);
		memcpy(m_data + kOffsetYPos, &yPos, 2);
		memcpy(m_data + kOffsetFinderFlags, &finderFlags, 2);
		memcpy(m_data + kOffsetProtected, &props.m_protected, 1);
		memcpy(m_data + kOffsetCreatedDate, &createdDate, 8);
		memcpy(m_data + kOffsetModifiedDate, &modifiedDate, 8);
	}

	bool MacFilePropertiesSerialized::WriteAsPackage(GpIOStream &stream, const CombinedTimestamp &ts) const
	{
		if (!WriteIsolated(stream, ts))
			return false;

		const char *packagedName = GetPackagedName();

		uint16_t msdosDate, msdosTime;
		ts.GetAsMSDOSTimestamp(msdosDate, msdosTime);

		ZipCentralDirectoryFileHeader cdh;
		cdh.m_signature = ZipCentralDirectoryFileHeader::kSignature;
		cdh.m_versionCreated = ZipConstants::kCompressedRequiredVersion;
		cdh.m_versionRequired = ZipConstants::kStoredRequiredVersion;
		cdh.m_flags = 0;
		cdh.m_method = ZipConstants::kStoredMethod;
		cdh.m_modificationTime = msdosTime;
		cdh.m_modificationDate = msdosDate;
		cdh.m_crc = PortabilityLayer::DeflateContext::CRC32(0, m_data, sizeof(m_data));
		cdh.m_compressedSize = sizeof(m_data);
		cdh.m_uncompressedSize = sizeof(m_data);
		cdh.m_fileNameLength = strlen(packagedName);
		cdh.m_extraFieldLength = 0;
		cdh.m_commentLength = 0;
		cdh.m_diskNumber = 0;
		cdh.m_internalAttributes = ZipConstants::kArchivedAttributes;
		cdh.m_externalAttributes = PortabilityLayer::ZipConstants::kArchivedAttributes;
		cdh.m_localHeaderOffset = 0;

		if (!stream.WriteExact(&cdh, sizeof(cdh)))
			return false;

		if (!stream.WriteExact(packagedName, strlen(packagedName)))
			return false;

		ZipEndOfCentralDirectoryRecord eod;
		eod.m_signature = ZipEndOfCentralDirectoryRecord::kSignature;
		eod.m_thisDiskNumber = 0;
		eod.m_centralDirDisk = 0;
		eod.m_numCentralDirRecordsThisDisk = 1;
		eod.m_numCentralDirRecords = 1;
		eod.m_centralDirectorySizeBytes = sizeof(ZipCentralDirectoryFileHeader) + strlen(packagedName);
		eod.m_centralDirStartOffset = sizeof(ZipFileLocalHeader) + strlen(packagedName) + sizeof(m_data);
		eod.m_commentLength = 0;

		if (stream.Write(&eod, sizeof(eod)) != sizeof(eod))
			return false;

		return true;
	}

	bool MacFilePropertiesSerialized::WriteIsolated(GpIOStream &stream, const CombinedTimestamp &ts) const
	{
		static const char *packagedName = GetPackagedName();

		ZipFileLocalHeader lh;
		static const uint32_t kSignature = 0x04034b50;

		uint16_t msdosDate, msdosTime;
		ts.GetAsMSDOSTimestamp(msdosDate, msdosTime);

		lh.m_signature = ZipFileLocalHeader::kSignature;
		lh.m_versionRequired = ZipConstants::kStoredRequiredVersion;
		lh.m_flags = 0;
		lh.m_method = ZipConstants::kStoredMethod;
		lh.m_modificationTime = msdosTime;
		lh.m_modificationDate = msdosDate;
		lh.m_crc = DeflateContext::CRC32(0, m_data, sizeof(m_data));
		lh.m_compressedSize = sizeof(m_data);
		lh.m_uncompressedSize = sizeof(m_data);
		lh.m_fileNameLength = strlen(packagedName);
		lh.m_extraFieldLength = 0;

		if (stream.Write(&lh, sizeof(lh)) != sizeof(lh))
			return false;

		if (stream.Write(packagedName, strlen(packagedName)) != strlen(packagedName))
			return false;

		if (stream.Write(m_data, sizeof(m_data)) != sizeof(m_data))
			return false;

		return true;
	}

	bool MacFilePropertiesSerialized::ReadFromPackage(GpIOStream &stream)
	{
		const char *packagedName = GetPackagedName();

		ZipFileLocalHeader lh;
		if (stream.Read(&lh, sizeof(lh)) != sizeof(lh))
			return false;

		if (lh.m_signature != ZipFileLocalHeader::kSignature)
			return false;
		if (lh.m_versionRequired != ZipConstants::kStoredRequiredVersion)
			return false;
		if (lh.m_flags != 0)
			return false;
		if (lh.m_method != ZipConstants::kStoredMethod)
			return false;

		if (lh.m_compressedSize != sizeof(m_data))
			return false;

		if (lh.m_uncompressedSize != sizeof(m_data))
			return false;

		if (lh.m_fileNameLength != strlen(packagedName))
			return false;

		if (lh.m_extraFieldLength != 0)
			return false;

		if (!stream.SeekCurrent(lh.m_fileNameLength))
			return false;

		if (stream.Read(m_data, sizeof(m_data)) != sizeof(m_data))
			return false;

		if (lh.m_crc != DeflateContext::CRC32(0, m_data, sizeof(m_data)))
			return false;

		return true;
	}

	const char *MacFilePropertiesSerialized::GetPackagedName()
	{
		return "!!meta";
	}
}
