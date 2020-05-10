#include "ArchiveDescription.h"

ArchiveCompressedChunkDesc::ArchiveCompressedChunkDesc()
	: m_filePosition(0)
	, m_uncompressedSize(0)
	, m_compressedSize(0)
	, m_compressionMethod(CompressionMethods::kUnknown)
{
}

ArchiveItem::ArchiveItem()
	: m_isDirectory(false)
	, m_children(nullptr)
{
}

ArchiveItem::ArchiveItem(ArchiveItem &&other)
	: m_isDirectory(other.m_isDirectory)
	, m_fileNameUTF8(other.m_fileNameUTF8)
	, m_macProperties(other.m_macProperties)
	, m_dataForkDesc(other.m_dataForkDesc)
	, m_resourceForkDesc(other.m_resourceForkDesc)
	, m_commentDesc(other.m_commentDesc)
	, m_children(other.m_children)
{
	other.m_children = nullptr;
}

ArchiveItem::ArchiveItem(const ArchiveItem &other)
	: m_isDirectory(other.m_isDirectory)
	, m_fileNameUTF8(other.m_fileNameUTF8)
	, m_macProperties(other.m_macProperties)
	, m_dataForkDesc(other.m_dataForkDesc)
	, m_resourceForkDesc(other.m_resourceForkDesc)
	, m_commentDesc(other.m_commentDesc)
	, m_children(other.m_children ? (new ArchiveItemList(*(other.m_children))) : nullptr)
{
}

ArchiveItem::~ArchiveItem()
{
	if (m_children)
		delete m_children;
}

ArchiveItemList::ArchiveItemList()
{
}

ArchiveItemList::ArchiveItemList(ArchiveItemList &&other)
	: m_items(std::move(other.m_items))
{
}

ArchiveItemList::ArchiveItemList(const ArchiveItemList &other)
	: m_items(other.m_items)
{
}

ArchiveItemList::~ArchiveItemList()
{
}
