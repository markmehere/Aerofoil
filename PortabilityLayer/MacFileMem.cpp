#include "MacFileMem.h"

namespace PortabilityLayer
{
	MacFileMem::MacFileMem(const uint8_t *dataFork, const uint8_t *resourceFork, const char* comment, const MacFileInfo &fileInfo)
		: m_info(fileInfo)
	{
		uint8_t *buffer = new uint8_t[fileInfo.m_dataForkSize + fileInfo.m_resourceForkSize + fileInfo.m_commentSize + 1];
		m_data.Set(buffer);

		memcpy(buffer, dataFork, fileInfo.m_dataForkSize);
		buffer += fileInfo.m_dataForkSize;

		memcpy(buffer, resourceFork, fileInfo.m_resourceForkSize);
		buffer += fileInfo.m_resourceForkSize;

		memcpy(buffer, comment, fileInfo.m_commentSize);
		buffer += fileInfo.m_commentSize;

		*buffer = 0;
	}

	MacFileMem::~MacFileMem()
	{
	}
}
