#include "StuffItParser.h"

#include "IFileReader.h"
#include "UPByteSwap.h"

#include "PLBigEndian.h"
#include "CRC.h"
#include "ArchiveDescription.h"
#include "StuffItCommon.h"
#include "StringCommon.h"

#include <vector>

bool StuffItParser::Check(IFileReader &reader)
{
	if (!reader.SeekStart(0))
		return false;

	uint8_t bytes[14];

	if (!reader.ReadExact(bytes, sizeof(bytes)))
		return false;


	if (ParseUInt32BE(bytes + 10) == 0x724c6175)
	{
		if (ParseUInt32BE(bytes) == 0x53495421)
			return true;

		// Installer archives?
		if (bytes[0] == 'S'&&bytes[1] == 'T')
		{
			if (bytes[2] == 'i' && (bytes[3] == 'n' || (bytes[3] >= '0'&&bytes[3] <= '9'))) return true;
			else if (bytes[2] >= '0'&&bytes[2] <= '9'&&bytes[3] >= '0'&&bytes[3] <= '9') return true;
		}
	}

	return false;
}

#define SITFH_COMPRMETHOD    0 /* xadUINT8 rsrc fork compression method */
#define SITFH_COMPDMETHOD    1 /* xadUINT8 data fork compression method */
#define SITFH_FNAMESIZE      2 /* xadUINT8 filename size */
#define SITFH_FNAME          3 /* xadUINT8 31 byte filename */
#define SITFH_FNAME_CRC     34 /* xadUINT16 crc of filename + size */

#define SITFH_UNK           36 /* xadUINT16 unknown, always 0x0986? */
#define SITFH_RSRCLONG      38 /* xadUINT32 unknown rsrc fork value */
#define SITFH_DATALONG      42 /* xadUINT32 unknown data fork value */
#define SITFH_DATACHAR      46 /* xadUINT8 unknown data (yes, data) fork value */
#define SITFH_RSRCCHAR      47 /* xadUINT8 unknown rsrc fork value */
#define SITFH_CHILDCOUNT    48 /* xadUINT16 number of items in dir */
#define SITFH_PREVOFFS      50 /* xadUINT32 offset of previous entry */
#define SITFH_NEXTOFFS      54 /* xadUINT32 offset of next entry */
#define SITFH_PARENTOFFS    58 /* xadUINT32 offset of parent entry */
#define SITFH_CHILDOFFS     62 /* xadINT32 offset of first child entry, -1 for file entries */

#define SITFH_FTYPE         66 /* xadUINT32 file type */
#define SITFH_CREATOR       70 /* xadUINT32 file creator */
#define SITFH_FNDRFLAGS     74 /* xadUINT16 Finder flags */
#define SITFH_CREATIONDATE  76 /* xadUINT32 creation date */
#define SITFH_MODDATE       80 /* xadUINT32 modification date */
#define SITFH_RSRCLENGTH    84 /* xadUINT32 decompressed rsrc length */
#define SITFH_DATALENGTH    88 /* xadUINT32 decompressed data length */
#define SITFH_COMPRLENGTH   92 /* xadUINT32 compressed rsrc length */
#define SITFH_COMPDLENGTH   96 /* xadUINT32 compressed data length */
#define SITFH_RSRCCRC      100 /* xadUINT16 crc of rsrc fork */
#define SITFH_DATACRC      102 /* xadUINT16 crc of data fork */

#define SITFH_RSRCPAD      104 /* xadUINT8 rsrc padding bytes for encryption */
#define SITFH_DATAPAD      105 /* xadUINT8 data padding bytes for encryption */
#define SITFH_DATAUNK1     106 /* xadUINT8 unknown data value, always 0? */
#define SITFH_DATAUNK2     107 /* xadUINT8 unknown data value, always 4 for encrypted? */
#define SITFH_RSRCUNK1     108 /* xadUINT8 unknown rsrc value, always 0? */
#define SITFH_RSRCUNK2     109 /* xadUINT8 unknown rsrc value, always 4 for encrypted? */

#define SITFH_HDRCRC       110 /* xadUINT16 crc of file header */
#define SIT_FILEHDRSIZE    112

#define StuffItEncryptedFlag 0x80 // password protected bit
#define StuffItStartFolder 0x20 // start of folder
#define StuffItEndFolder 0x21 // end of folder
#define StuffItFolderContainsEncrypted 0x10 // folder contains encrypted items bit
#define StuffItMethodMask (~StuffItEncryptedFlag)
#define StuffItFolderMask (~(StuffItEncryptedFlag|StuffItFolderContainsEncrypted))

struct StuffItHeader
{
	uint8_t m_signature[4];
	BEUInt16_t m_numRootFiles;
	BEUInt32_t m_totalSize;
	uint8_t m_signature2[4];
	uint8_t m_version;
	uint8_t m_reserved;
	BEUInt32_t m_headerSize;	// In version 1, this is absent, header size is always 22
	BEUInt16_t m_crc;
};

void ParseMacFileInfo(ArchiveItem &item, const uint8_t *header, IFileReader::FilePos_t filePos)
{
	item.m_macProperties.m_creationDate = ParseUInt32BE(header + SITFH_CREATIONDATE);
	item.m_macProperties.m_modifiedDate = ParseUInt32BE(header + SITFH_MODDATE);
	memcpy(item.m_macProperties.m_fileCreator, header + SITFH_CREATOR, 4);
	memcpy(item.m_macProperties.m_fileType, header + SITFH_FTYPE, 4);
	item.m_macProperties.m_finderFlags = ParseUInt16BE(header + SITFH_FNDRFLAGS);
}

ArchiveItemList *StuffItParser::Parse(IFileReader &reader)
{
	if (!reader.SeekStart(0))
		return nullptr;

	IFileReader::FilePos_t base = reader.GetPosition();

	StuffItHeader archiveHeader;
	if (!reader.ReadExact(&archiveHeader, sizeof(archiveHeader)))
		return nullptr;

	uint32_t headerSize = 0;

	if (archiveHeader.m_version == 0)
		return nullptr;
	else if (archiveHeader.m_version == 1)
		headerSize = sizeof(archiveHeader);
	else
		headerSize = archiveHeader.m_headerSize;

	if (headerSize < sizeof(archiveHeader))
		return nullptr;

	reader.SeekCurrent(headerSize - sizeof(archiveHeader));

	IFileReader::FilePos_t totalsize = archiveHeader.m_totalSize;
	if (static_cast<IFileReader::FilePos_t>(totalsize) > static_cast<IFileReader::FilePos_t>(reader.FileSize()))
		return nullptr;

	ArchiveItemList rootList;

	std::vector<ArchiveItemList*> folderStack;
	folderStack.push_back(&rootList);

	while (reader.GetPosition() + SIT_FILEHDRSIZE <= totalsize + base)
	{
		uint8_t header[SIT_FILEHDRSIZE];
		if (!reader.ReadExact(header, SIT_FILEHDRSIZE))
			return nullptr;

		if (ParseUInt16BE(header + SITFH_HDRCRC) == XADCalculateCRC(0, header, 110, XADCRCTable_a001))
		{
			int resourcelength = ParseUInt32BE(header + SITFH_RSRCLENGTH);
			int resourcecomplen = ParseUInt32BE(header + SITFH_COMPRLENGTH);
			int datalength = ParseUInt32BE(header + SITFH_DATALENGTH);
			int datacomplen = ParseUInt32BE(header + SITFH_COMPDLENGTH);
			int datamethod = header[SITFH_COMPDMETHOD];
			int resourcemethod = header[SITFH_COMPRMETHOD];
			int datapadding = header[SITFH_DATAPAD];
			int resourcepadding = header[SITFH_RSRCPAD];

			int namelen = header[SITFH_FNAMESIZE];
			if (namelen > 31) namelen = 31;

			const uint8_t fileNameLength = header[SITFH_FNAMESIZE];
			if (fileNameLength > SITFH_FNAME_CRC - SITFH_FNAME)
				return false;

			std::vector<uint8_t> utf8FileName;
			StringCommon::ConvertMacRomanFileName(utf8FileName, header + SITFH_FNAME, fileNameLength);

			IFileReader::FilePos_t start = reader.GetPosition();

			ArchiveItemList *stackTop = folderStack.back();
			stackTop->m_items.push_back(ArchiveItem());

			ArchiveItem &addedItem = stackTop->m_items.back();
			addedItem.m_fileNameUTF8 = std::move(utf8FileName);

			if ((datamethod&StuffItFolderMask) == StuffItStartFolder ||
				(resourcemethod&StuffItFolderMask) == StuffItStartFolder)
			{
				addedItem.m_isDirectory = true;
				ParseMacFileInfo(addedItem, header, start);

				if ((datamethod&StuffItFolderContainsEncrypted) != 0 ||
					(resourcemethod&StuffItFolderContainsEncrypted) != 0)
				{
					return nullptr;
				}

				addedItem.m_children = new ArchiveItemList();
				folderStack.push_back(addedItem.m_children);
			}
			else if ((datamethod&StuffItFolderMask) == StuffItEndFolder ||
				(resourcemethod&StuffItFolderMask) == StuffItEndFolder)
			{
				if (folderStack.size() == 1)
					return nullptr;

				stackTop->m_items.pop_back();

				folderStack.pop_back();
			}
			else
			{
				ParseMacFileInfo(addedItem, header, start);

				if (resourcelength)
				{
					if (resourcemethod & StuffItEncryptedFlag)
						return nullptr;

					addedItem.m_resourceForkDesc.m_compressedSize = resourcecomplen;
					addedItem.m_resourceForkDesc.m_uncompressedSize = resourcelength;
					addedItem.m_resourceForkDesc.m_filePosition = start;
					addedItem.m_resourceForkDesc.m_compressionMethod = StuffItCommon::ResolveCompressionMethod(resourcemethod & StuffItMethodMask);
				}

				if (datalength)
				{
					if (datamethod & StuffItEncryptedFlag)
						return nullptr;

					addedItem.m_dataForkDesc.m_compressedSize = resourcecomplen;
					addedItem.m_dataForkDesc.m_uncompressedSize = datalength;
					addedItem.m_dataForkDesc.m_filePosition = start + resourcecomplen;
					addedItem.m_dataForkDesc.m_compressionMethod = StuffItCommon::ResolveCompressionMethod(datamethod & StuffItMethodMask);
				}

				if (!reader.SeekCurrent(resourcecomplen + datacomplen))
					return nullptr;
			}
		}
		else
			return nullptr;
	}

	if (folderStack.size() != 1)
		return nullptr;

	return new ArchiveItemList(std::move(rootList));
}
