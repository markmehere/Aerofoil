#include "StuffIt5Parser.h"
#include "UPByteSwap.h"
#include "IFileReader.h"
#include "ArchiveDescription.h"
#include "StuffItCommon.h"
#include "PLBigEndian.h"

#include <vector>

#include "CSInputBuffer.h"

struct StuffIt5Header
{
	uint8_t m_signature[80];
	uint8_t m_unknown1[4];
	BEUInt32_t m_fileSize;
	uint8_t m_unknown2[4];
	BEUInt16_t m_numRootDirEntries;
	BEUInt32_t m_rootDirFirstEntryOffset;
};

struct StuffIt5DataBlockDesc
{
	BEUInt32_t m_uncompressedSize;
	BEUInt32_t m_compressedSize;
	BEUInt16_t m_crc;
	uint8_t m_unknown[2];
	uint8_t m_algorithm_dirNumFilesHigh;
	uint8_t m_passwordDataLength_dirNumFilesLow;
};

struct StuffIt5BlockHeader
{
	uint8_t m_identifier[4];
	uint8_t m_version;
	uint8_t m_unknown1;
	BEUInt16_t m_headerSize;
	uint8_t m_unknown2;
	uint8_t m_flags;
	BEUInt32_t m_creationDate;
	BEUInt32_t m_modificationDate;
	BEUInt32_t m_prevEntryOffset;
	BEUInt32_t m_nextEntryOffset;
	BEUInt32_t m_dirEntryOffset;
	BEUInt16_t m_fileNameSize;
	BEUInt16_t m_headerCRC;

	StuffIt5DataBlockDesc m_dataForkDesc;

	// Followed by:
	// uint8_t passwordData[m_passwordDataLength_dirNumFilesLow]
	// uint8_t filename[m_fileNameSize]
	// BEUInt16 commentSize
	// uint8_t unknown4[2]
	// Comment[commentSize]
};

struct StuffIt5BlockAnnex1
{
	BEUInt16_t m_unknown1;	// Low bit = has resource fork
	BEUInt16_t m_unknown2;
	uint8_t m_fileType[4];
	uint8_t m_fileCreator[4];
	BEUInt16_t m_finderFlags;
};


#define SIT5_ID 0xA5A5A5A5

#define SIT5FLAGS_DIRECTORY     0x40
#define SIT5FLAGS_CRYPTED       0x20
#define SIT5FLAGS_RSRC_FORK     0x10

#define SIT5_ARCHIVEVERSION 5

#define SIT5_ARCHIVEFLAGS_14BYTES 0x10
#define SIT5_ARCHIVEFLAGS_20      0x20
#define SIT5_ARCHIVEFLAGS_40      0x40
#define SIT5_ARCHIVEFLAGS_CRYPTED 0x80

#define SIT5_KEY_LENGTH 5  /* 40 bits */

struct StuffIt5Block
{
	StuffIt5BlockHeader m_header;
	StuffIt5BlockAnnex1 m_annex1;

	bool m_isDirectory;

	bool m_hasResourceFork;
	StuffIt5DataBlockDesc m_resourceForkDesc;
	std::vector<uint8_t> m_resourceForkPasswordData;

	IFileReader::FilePos_t m_dataForkPos;
	IFileReader::FilePos_t m_resForkPos;

	uint16_t m_commentSize;
	IFileReader::FilePos_t m_commentPos;

	std::vector<uint8_t> m_passwordData;
	std::vector<uint8_t> m_filename;

	std::vector<StuffIt5Block> m_children;

	bool Read(IFileReader &reader)
	{
		if (!reader.ReadExact(&m_header, sizeof(m_header)))
			return false;

		uint32_t sizeWithOnlyNameAndPasswordInfo = sizeof(m_header);

		m_isDirectory = ((m_header.m_flags & SIT5FLAGS_DIRECTORY) != 0);

		if (!m_isDirectory && m_header.m_dataForkDesc.m_passwordDataLength_dirNumFilesLow != 0)
		{
			m_passwordData.resize(m_header.m_dataForkDesc.m_passwordDataLength_dirNumFilesLow);
			if (!reader.ReadExact(&m_passwordData[0], m_header.m_dataForkDesc.m_passwordDataLength_dirNumFilesLow))
				return false;

			sizeWithOnlyNameAndPasswordInfo += m_header.m_dataForkDesc.m_passwordDataLength_dirNumFilesLow;
		}

		if (m_header.m_fileNameSize)
		{
			m_filename.resize(m_header.m_fileNameSize);
			if (!reader.ReadExact(&m_filename[0], m_header.m_fileNameSize))
				return false;

			sizeWithOnlyNameAndPasswordInfo += m_header.m_fileNameSize;
		}

		uint32_t sizeWithCommentData = sizeWithOnlyNameAndPasswordInfo;

		if (sizeWithOnlyNameAndPasswordInfo != m_header.m_headerSize)
		{
			if (sizeWithOnlyNameAndPasswordInfo > m_header.m_headerSize || m_header.m_headerSize - sizeWithOnlyNameAndPasswordInfo < 4)
				return false;

			BEUInt16_t extras[2];
			if (!reader.ReadExact(extras, sizeof(extras)))
				return false;

			uint16_t commentLength = extras[0];

			if (commentLength > m_header.m_headerSize - sizeWithOnlyNameAndPasswordInfo - 4)
				return false;


			m_commentSize = commentLength;
			m_commentPos = reader.GetPosition();

			if (commentLength)
			{
				if (reader.SeekCurrent(commentLength))
					return false;
			}

			sizeWithCommentData += commentLength + 4;
		}
		else
		{
			m_commentSize = 0;
			m_commentPos = 0;
		}

		if (!reader.SeekCurrent(m_header.m_headerSize - sizeWithCommentData))
			return false;

		if (!reader.ReadExact(&m_annex1, sizeof(m_annex1)))
			return false;

		if (m_header.m_version == 1)
		{
			if (!reader.SeekCurrent(22))
				return false;
		}
		else
		{
			if (!reader.SeekCurrent(18))
				return false;
		}

		m_hasResourceFork = ((m_annex1.m_unknown1 & 0x01) != 0);

		if (m_hasResourceFork)
		{
			if (!reader.ReadExact(&m_resourceForkDesc, sizeof(m_resourceForkDesc)))
				return false;

			if (m_resourceForkDesc.m_passwordDataLength_dirNumFilesLow)
			{
				m_resourceForkPasswordData.resize(m_resourceForkDesc.m_passwordDataLength_dirNumFilesLow);
				if (!reader.ReadExact(&m_resourceForkPasswordData[0], m_resourceForkDesc.m_passwordDataLength_dirNumFilesLow))
					return false;
			}
		}

		if (m_isDirectory)
		{
			int numFiles = (m_header.m_dataForkDesc.m_algorithm_dirNumFilesHigh << 8) | (m_header.m_dataForkDesc.m_passwordDataLength_dirNumFilesLow);

			m_children.resize(numFiles);
			for (int i = 0; i < numFiles; i++)
			{
				if (i != 0)
				{
					if (!reader.SeekStart(m_children[i - 1].m_header.m_nextEntryOffset))
						return false;
				}

				if (!m_children[i].Read(reader))
					return false;
			}
		}
		else
		{
			if (m_hasResourceFork)
			{
				m_resForkPos = reader.GetPosition();
				m_dataForkPos = m_resForkPos + m_resourceForkDesc.m_compressedSize;
			}
			else
				m_dataForkPos = reader.GetPosition();
		}

		return true;
	}
};

static ArchiveItemList *ConvertToItemList(const std::vector<StuffIt5Block> &blocks);

static void ConvertItem(const StuffIt5Block &block, ArchiveItem &item)
{
	item.m_isDirectory = block.m_isDirectory;

	if (block.m_isDirectory)
		item.m_children = ConvertToItemList(block.m_children);
	else
	{
		item.m_macProperties.m_creationDate = block.m_header.m_creationDate;
		item.m_macProperties.m_modifiedDate = block.m_header.m_modificationDate;
		memcpy(item.m_macProperties.m_fileCreator, block.m_annex1.m_fileCreator, 4);
		memcpy(item.m_macProperties.m_fileType, block.m_annex1.m_fileType, 4);
		item.m_macProperties.m_finderFlags = block.m_annex1.m_finderFlags;

		item.m_dataForkDesc.m_filePosition = block.m_dataForkPos;
		item.m_dataForkDesc.m_compressedSize = block.m_header.m_dataForkDesc.m_compressedSize;
		item.m_dataForkDesc.m_uncompressedSize = block.m_header.m_dataForkDesc.m_uncompressedSize;
		item.m_dataForkDesc.m_compressionMethod = StuffItCommon::ResolveCompressionMethod(block.m_header.m_dataForkDesc.m_algorithm_dirNumFilesHigh);

		if (block.m_hasResourceFork)
		{
			item.m_resourceForkDesc.m_filePosition = block.m_resForkPos;
			item.m_resourceForkDesc.m_compressedSize = block.m_resourceForkDesc.m_compressedSize;
			item.m_resourceForkDesc.m_uncompressedSize = block.m_resourceForkDesc.m_uncompressedSize;
			item.m_resourceForkDesc.m_compressionMethod = StuffItCommon::ResolveCompressionMethod(block.m_resourceForkDesc.m_algorithm_dirNumFilesHigh);
		}
	}

	item.m_fileNameUTF8 = block.m_filename;

	item.m_commentDesc.m_compressedSize = block.m_commentSize;
	item.m_commentDesc.m_uncompressedSize = block.m_commentSize;
	item.m_commentDesc.m_filePosition = block.m_commentPos;
	item.m_commentDesc.m_compressionMethod = CompressionMethods::kNone;
}

static ArchiveItemList *ConvertToItemList(const std::vector<StuffIt5Block> &blocks)
{
	ArchiveItemList *list = new ArchiveItemList();
	const size_t numItems = blocks.size();

	list->m_items.resize(numItems);
	for (size_t i = 0; i < numItems; i++)
	{
		ConvertItem(blocks[i], list->m_items[i]);
	}

	return list;
}

bool StuffIt5Parser::Check(IFileReader &reader)
{
	char signatureBytes[80];

	if (reader.FileSize() < sizeof(signatureBytes))
		return false;

	if (!reader.SeekStart(0))
		return false;

	if (!reader.ReadExact(signatureBytes, sizeof(signatureBytes)))
		return false;

	const char *match = "StuffIt (c)1997-\xFF\xFF\xFF\xFF Aladdin Systems, Inc., http://www.aladdinsys.com/StuffIt/\x0d\x0a";

	const char *checkByte = signatureBytes;
	while (*match && (*checkByte == *match || *match == '\377'))
	{
		match++;
		checkByte++;
	}

	return (*match) == '\0';
}

ArchiveItemList *StuffIt5Parser::Parse(IFileReader &reader)
{
	reader.SeekStart(0);

	StuffIt5Header header;
	if (!reader.Read(&header, sizeof(header)))
		return nullptr;

	uint16_t numRootDirEntries = header.m_numRootDirEntries;

	if (!reader.SeekStart(header.m_rootDirFirstEntryOffset))
		return nullptr;

	std::vector<StuffIt5Block> rootDirBlocks;
	rootDirBlocks.resize(numRootDirEntries);

	for (int i = 0; i < numRootDirEntries; i++)
	{
		if (i != 0)
			reader.SeekStart(rootDirBlocks[i - 1].m_header.m_nextEntryOffset);

		if (!rootDirBlocks[i].Read(reader))
			return nullptr;
	}

	return ConvertToItemList(rootDirBlocks);
}
