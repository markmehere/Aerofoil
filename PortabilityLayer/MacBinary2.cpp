#include "MacBinary2.h"

#include "BytePack.h"
#include "ByteUnpack.h"
#include "DataTypes.h"
#include "GpIOStream.h"
#include "MacFileMem.h"
#include "XModemCRC.h"

// See: https://files.stairways.com/other/macbinaryii-standard-info.txt

namespace
{
	namespace MB2FileOffsets
	{
		const unsigned int Version = 0;
		const unsigned int FileNameLength = 1;
		const unsigned int FileName = 2;
		const unsigned int FileType = 65;
		const unsigned int FileCreator = 69;
		const unsigned int FinderFlagsHigh = 73;
		const unsigned int YPos = 75;
		const unsigned int XPos = 77;
		const unsigned int Protected = 81;
		const unsigned int DataForkSize = 83;
		const unsigned int ResourceForkSize = 87;
		const unsigned int CreatedDate = 91;
		const unsigned int ModifiedDate = 95;
		const unsigned int CommentLength = 99;
		const unsigned int FinderFlagsLow = 101;
		const unsigned int DecompressedSize = 116;
		const unsigned int SecondaryHeaderLength = 120;

		const unsigned int WriterVersion = 122;
		const unsigned int MinVersion = 123;
		const unsigned int Checksum = 124;
		const unsigned int ContentStart = 128;
	};
}

namespace PortabilityLayer
{
	void MacBinary2::WriteBin(const MacFileMem *file, GpIOStream *stream)
	{
		const MacFileInfo &fileInfo = file->FileInfo();

		uint8_t mb2Header[128];

		memset(mb2Header, 0, sizeof(mb2Header));

		mb2Header[MB2FileOffsets::Version] = 0;

		size_t fileNameLength = fileInfo.m_fileName.Length();
		if (fileNameLength == 0)
		{
			mb2Header[MB2FileOffsets::FileNameLength] = 1;
			mb2Header[MB2FileOffsets::FileName] = '?';
		}
		else
		{
			if (fileNameLength > 63)
				fileNameLength = 63;

			mb2Header[MB2FileOffsets::FileNameLength] = static_cast<uint8_t>(fileNameLength);
			memcpy(mb2Header + MB2FileOffsets::FileName, &fileInfo.m_fileName[0], fileNameLength);
		}

		memcpy(mb2Header + MB2FileOffsets::FileType, fileInfo.m_properties.m_fileType, 4);
		memcpy(mb2Header + MB2FileOffsets::FileCreator, fileInfo.m_properties.m_fileCreator, 4);

		mb2Header[MB2FileOffsets::FinderFlagsHigh] = static_cast<uint8_t>((fileInfo.m_properties.m_finderFlags >> 8) & 0xff);

		BytePack::BigInt16(mb2Header + MB2FileOffsets::YPos, fileInfo.m_properties.m_yPos);
		BytePack::BigInt16(mb2Header + MB2FileOffsets::XPos, fileInfo.m_properties.m_xPos);
		mb2Header[MB2FileOffsets::Protected] = fileInfo.m_properties.m_protected;
		BytePack::BigUInt32(mb2Header + MB2FileOffsets::DataForkSize, fileInfo.m_dataForkSize);
		BytePack::BigUInt32(mb2Header + MB2FileOffsets::ResourceForkSize, fileInfo.m_resourceForkSize);
		BytePack::BigUInt32(mb2Header + MB2FileOffsets::CreatedDate, static_cast<uint32_t>(fileInfo.m_properties.m_createdTimeMacEpoch));
		BytePack::BigUInt32(mb2Header + MB2FileOffsets::ModifiedDate, static_cast<uint32_t>(fileInfo.m_properties.m_modifiedTimeMacEpoch));

		BytePack::BigUInt16(mb2Header + MB2FileOffsets::CommentLength, fileInfo.m_commentSize);
		mb2Header[MB2FileOffsets::FinderFlagsLow] = static_cast<uint8_t>(fileInfo.m_properties.m_finderFlags & 0xff);
		// DecompressedSize is unused
		// SecondaryHeaderLength is zero

		mb2Header[MB2FileOffsets::WriterVersion] = 129;
		mb2Header[MB2FileOffsets::MinVersion] = 129;

		BytePack::BigUInt16(mb2Header + MB2FileOffsets::Checksum, XModemCRC(mb2Header, 124, 0));

		stream->Write(mb2Header, 128);

		uint8_t *padding = mb2Header;
		memset(padding, 0, 128);

		const size_t dataForkPadding = 127 - ((fileInfo.m_dataForkSize + 127) % 128);
		const size_t resourceForkPadding = 127 - ((fileInfo.m_resourceForkSize + 127) % 128);

		stream->Write(file->DataFork(), fileInfo.m_dataForkSize);
		stream->Write(padding, dataForkPadding);
		stream->Write(file->ResourceFork(), fileInfo.m_resourceForkSize);
		stream->Write(padding, resourceForkPadding);
	}

	MacFileMem *MacBinary2::ReadBin(GpIOStream *stream)
	{
		MacFileInfo fileInfo;

		uint8_t mb2Header[128];

		if (stream->Read(mb2Header, 128) != 128)
			return nullptr;

		if (mb2Header[MB2FileOffsets::Version] != 0)
			return nullptr;

		const uint8_t fileNameLength = mb2Header[MB2FileOffsets::FileNameLength];
		if (fileNameLength < 1 || fileNameLength > 63)
			return nullptr;

		fileInfo.m_fileName.Set(fileNameLength, reinterpret_cast<const char*>(mb2Header + MB2FileOffsets::FileName));

		memcpy(fileInfo.m_properties.m_fileType, mb2Header + MB2FileOffsets::FileType, 4);
		memcpy(fileInfo.m_properties.m_fileCreator, mb2Header + MB2FileOffsets::FileCreator, 4);

		fileInfo.m_properties.m_finderFlags = mb2Header[MB2FileOffsets::FinderFlagsHigh] << 8;

		fileInfo.m_properties.m_yPos = ByteUnpack::BigInt16(mb2Header + MB2FileOffsets::YPos);
		fileInfo.m_properties.m_xPos = ByteUnpack::BigInt16(mb2Header + MB2FileOffsets::XPos);
		fileInfo.m_properties.m_protected = mb2Header[MB2FileOffsets::Protected];
		fileInfo.m_dataForkSize = ByteUnpack::BigUInt32(mb2Header + MB2FileOffsets::DataForkSize);
		fileInfo.m_resourceForkSize = ByteUnpack::BigUInt32(mb2Header + MB2FileOffsets::ResourceForkSize);
		fileInfo.m_properties.m_createdTimeMacEpoch = ByteUnpack::BigUInt32(mb2Header + MB2FileOffsets::CreatedDate);
		fileInfo.m_properties.m_modifiedTimeMacEpoch = ByteUnpack::BigUInt32(mb2Header + MB2FileOffsets::ModifiedDate);

		fileInfo.m_commentSize = ByteUnpack::BigUInt16(mb2Header + MB2FileOffsets::CommentLength);
		fileInfo.m_properties.m_finderFlags |= mb2Header[MB2FileOffsets::FinderFlagsLow];

		if (ByteUnpack::BigInt16(mb2Header + MB2FileOffsets::SecondaryHeaderLength) != 0)
			return nullptr;

		uint16_t crc = ByteUnpack::BigUInt16(mb2Header + MB2FileOffsets::Checksum);

		uint16_t expectedCRC = XModemCRC(mb2Header, 124, 0);

		if (fileInfo.m_dataForkSize > SIZE_MAX)
			return nullptr;

		if (fileInfo.m_resourceForkSize > SIZE_MAX)
			return nullptr;

		uint8_t *dataBuffer = nullptr;
		uint8_t *rsrcBuffer = nullptr;

		if (fileInfo.m_dataForkSize != 0)
			dataBuffer = new uint8_t[fileInfo.m_dataForkSize];

		if (fileInfo.m_resourceForkSize != 0)
			rsrcBuffer = new uint8_t[fileInfo.m_resourceForkSize];

		ScopedArray<uint8_t> dataContents(dataBuffer);
		ScopedArray<uint8_t> rsrcContents(rsrcBuffer);

		uint8_t *padding = mb2Header;

		const size_t dataForkPadding = 127 - ((fileInfo.m_dataForkSize + 127) % 128);
		const size_t resourceForkPadding = 127 - ((fileInfo.m_resourceForkSize + 127) % 128);

		if (stream->Read(dataBuffer, fileInfo.m_dataForkSize) != fileInfo.m_dataForkSize)
			return nullptr;

		if (stream->Read(padding, dataForkPadding) != dataForkPadding)
			return nullptr;

		if (stream->Read(rsrcBuffer, fileInfo.m_resourceForkSize) != fileInfo.m_resourceForkSize)
			return nullptr;

		if (stream->Read(padding, resourceForkPadding) != resourceForkPadding)
			return nullptr;

		// Ignore comment for now
		return new MacFileMem(dataBuffer, rsrcBuffer, nullptr, fileInfo);
	}
}

