#include "CompactProParser.h"

#include "IFileReader.h"
#include "UPByteSwap.h"
#include "PLBigEndian.h"
#include "CRC.h"

#include "ArchiveDescription.h"
#include "StringCommon.h"

#include <stdint.h>

bool CompactProParser::Check(IFileReader &reader)
{
	uint8_t bytes[8];

	if (!reader.SeekStart(0))
		return false;

	if (!reader.ReadExact(bytes, sizeof(bytes)))
		return false;

	if (bytes[0] != 1)
		return false;

	uint32_t offset = ParseUInt32BE(bytes + 4);

	if (offset > reader.FileSize() - 7)
		return false;

	if (!reader.SeekStart(offset))
		return false;

	uint8_t buf[256];

	if (!reader.ReadExact(buf, 4))
		return false;

	uint32_t correctcrc = ParseUInt32BE(buf);

	uint32_t crc = 0xffffffff;
	if (!reader.ReadExact(buf, 3))
		return false;

	crc = XADCalculateCRC(crc, buf, 3, XADCRCTable_edb88320);

	int numentries = ParseUInt16BE(buf);
	int commentsize = buf[2];

	if (!reader.ReadExact(buf, commentsize))
		return false;

	crc = XADCalculateCRC(crc, buf, commentsize, XADCRCTable_edb88320);

	for (int i = 0; i < numentries; i++)
	{
		uint8_t namelen;
		if (!reader.ReadExact(&namelen, 1))
			return false;

		crc = XADCRC(crc, namelen, XADCRCTable_edb88320);

		if (!reader.ReadExact(buf, namelen & 0x7f))
			return false;

		crc = XADCalculateCRC(crc, buf, namelen & 0x7f, XADCRCTable_edb88320);

		int metadatasize;
		if (namelen & 0x80)
			metadatasize = 2;
		else
			metadatasize = 45;

		if (!reader.ReadExact(buf, metadatasize))
			return false;

		crc = XADCalculateCRC(crc, buf, metadatasize, XADCRCTable_edb88320);
	}

	if (crc == correctcrc)
		return true;

	return false;
}

struct CompactProArchiveHeader
{
	uint8_t m_marker;
	uint8_t m_volume;
	BEUInt16_t m_xmagic;
	BEUInt32_t m_catalogOffset;
};

struct CompactProCatalogHeader
{
	BEUInt32_t m_headerCRC;
	BEUInt16_t m_numEntries;
	uint8_t m_commentLength;
};

struct CompactProCatalogFileEntry
{
	uint8_t m_volume;
	BEUInt32_t m_fileOffset;
	uint8_t m_fileType[4];
	uint8_t m_fileCreator[4];
	BEUInt32_t m_creationDate;
	BEUInt32_t m_modificationDate;
	BEUInt16_t m_finderFlags;
	BEUInt32_t m_crc;
	BEUInt16_t m_flags;

	BEUInt32_t m_resUncompressedSize;
	BEUInt32_t m_dataUncompressedSize;
	BEUInt32_t m_resCompressedSize;
	BEUInt32_t m_dataCompressedSize;
};

ArchiveItemList *CompactProParser::Parse(IFileReader &reader)
{
	if (!reader.SeekStart(0))
		return nullptr;

	CompactProArchiveHeader arcHeader;
	if (!reader.ReadExact(&arcHeader, sizeof(arcHeader)))
		return nullptr;

	if (!reader.SeekStart(arcHeader.m_catalogOffset))
		return nullptr;

	CompactProCatalogHeader catHeader;
	if (!reader.ReadExact(&catHeader, sizeof(catHeader)))
		return nullptr;

	if (!reader.SeekCurrent(catHeader.m_commentLength))
		return nullptr;

	return ParseDirectory(catHeader.m_numEntries, reader);
}

ArchiveItemList *CompactProParser::ParseDirectory(uint32_t numEntries, IFileReader &reader)
{
	ArchiveItemList itemList;

	for (uint32_t itemIndex = 0; itemIndex < numEntries; itemIndex++)
	{
		itemList.m_items.push_back(ArchiveItem());
		ArchiveItem &item = itemList.m_items.back();

		uint8_t nameLengthAndDirFlag;
		if (!reader.ReadExact(&nameLengthAndDirFlag, 1))
			return nullptr;

		uint8_t nameLength = (nameLengthAndDirFlag & 0x7f);

		uint8_t fname[127];
		if (!reader.ReadExact(fname, nameLength))
			return nullptr;

		StringCommon::ConvertMacRomanFileName(item.m_fileNameUTF8, fname, nameLength);

		if (nameLengthAndDirFlag & 0x80)
		{
			BEUInt16_t numChildrenBE;
			if (!reader.ReadExact(&numChildrenBE, sizeof(numChildrenBE)))
				return nullptr;

			uint16_t numChildren = numChildrenBE;

			if (numChildren > numEntries - itemIndex)
				return nullptr;

			item.m_isDirectory = true;

			ArchiveItemList *children = ParseDirectory(numChildren, reader);
			if (!children)
				return nullptr;

			item.m_children = children;
			numEntries -= numChildren;
		}
		else
		{
			CompactProCatalogFileEntry fileEntry;
			if (!reader.ReadExact(&fileEntry, sizeof(fileEntry)))
				return nullptr;

			if (fileEntry.m_resUncompressedSize)
			{
				item.m_resourceForkDesc.m_compressedSize = fileEntry.m_resCompressedSize;
				item.m_resourceForkDesc.m_uncompressedSize = fileEntry.m_resUncompressedSize;
				item.m_resourceForkDesc.m_filePosition = fileEntry.m_fileOffset;
				item.m_resourceForkDesc.m_compressionMethod = (fileEntry.m_flags & 2) ? CompressionMethods::kCompactProLZHRLE : CompressionMethods::kCompactProRLE;
			}

			if (fileEntry.m_dataUncompressedSize)
			{
				item.m_dataForkDesc.m_compressedSize = fileEntry.m_dataCompressedSize;
				item.m_dataForkDesc.m_uncompressedSize = fileEntry.m_dataUncompressedSize;
				item.m_dataForkDesc.m_filePosition = fileEntry.m_fileOffset + fileEntry.m_resCompressedSize;
				item.m_dataForkDesc.m_compressionMethod = (fileEntry.m_flags & 4) ? CompressionMethods::kCompactProLZHRLE : CompressionMethods::kCompactProRLE;
			}

			item.m_macProperties.m_createdTimeMacEpoch = fileEntry.m_creationDate;
			item.m_macProperties.m_modifiedTimeMacEpoch = fileEntry.m_modificationDate;
			memcpy(item.m_macProperties.m_fileCreator, fileEntry.m_fileCreator, 4);
			memcpy(item.m_macProperties.m_fileType, fileEntry.m_fileType, 4);
			item.m_macProperties.m_finderFlags = fileEntry.m_finderFlags;
		}
	}

	return new ArchiveItemList(std::move(itemList));
}
