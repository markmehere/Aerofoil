#pragma once

#include "MacFileInfo.h"

#include <vector>
#include <stdint.h>

namespace CompressionMethods
{
	enum CompressionMethod
	{
		kNone,
		kUnknown,

		kStuffItRLE90,
		kStuffItLZW,
		kStuffItHuffman,
		kStuffItLZAH,
		kStuffItFixedHuffman,
		kStuffItMW,
		kStuffIt13,
		kStuffIt14,
		kStuffItArsenic,

		kCompactProRLE,
		kCompactProLZHRLE,
	};
}

typedef CompressionMethods::CompressionMethod CompressionMethod_t;

struct ArchiveItemList;

struct ArchiveCompressedChunkDesc
{
	ArchiveCompressedChunkDesc();

	uint64_t m_filePosition;
	size_t m_uncompressedSize;
	size_t m_compressedSize;
	CompressionMethod_t m_compressionMethod;
};

struct ArchiveItem
{
	ArchiveItem();
	ArchiveItem(ArchiveItem &&other);
	ArchiveItem(const ArchiveItem &other);
	~ArchiveItem();

	bool m_isDirectory;
	std::vector<uint8_t> m_fileNameUTF8;
	PortabilityLayer::MacFileProperties m_macProperties;

	ArchiveCompressedChunkDesc m_dataForkDesc;
	ArchiveCompressedChunkDesc m_resourceForkDesc;
	ArchiveCompressedChunkDesc m_commentDesc;

	ArchiveItemList *m_children;
};

struct ArchiveItemList
{
public:
	ArchiveItemList();
	ArchiveItemList(ArchiveItemList &&other);
	ArchiveItemList(const ArchiveItemList &other);
	~ArchiveItemList();

	std::vector<ArchiveItem> m_items;
};
