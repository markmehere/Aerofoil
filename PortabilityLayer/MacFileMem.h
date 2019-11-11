#pragma once

#ifndef __PL_MACFILEMEM_H__
#define __PL_MACFILEMEM_H__

#include "DataTypes.h"
#include "MacFileInfo.h"
#include "ScopedArray.h"

namespace PortabilityLayer
{
	class MacFileMem
	{
	public:
		MacFileMem(const uint8_t *dataFork, const uint8_t *resourceFork, const char* comment, const MacFileInfo &fileInfo);
		~MacFileMem();

		const MacFileInfo &FileInfo() const;
		const uint8_t *DataFork() const;
		const uint8_t *ResourceFork() const;
		const char *Comment() const;

	private:
		ScopedArray<uint8_t> m_data;
		MacFileInfo m_info;
	};
}

namespace PortabilityLayer
{
	inline const MacFileInfo &MacFileMem::FileInfo() const
	{
		return m_info;
	}

	inline const uint8_t *MacFileMem::DataFork() const
	{
		return m_data;
	}

	inline const uint8_t *MacFileMem::ResourceFork() const
	{
		return m_data + m_info.m_dataForkSize;
	}

	inline const char *MacFileMem::Comment() const
	{
		return reinterpret_cast<const char*>(m_data + m_info.m_dataForkSize + m_info.m_resourceForkSize);
	}
}

#endif
