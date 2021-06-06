#pragma once

#include "DataTypes.h"
#include "PascalStr.h"

class GpIOStream;

namespace PortabilityLayer
{
	struct CombinedTimestamp;

	enum FinderFileFlags
	{
		FINDER_FILE_FLAG_ALIAS = (1 << 15),
		FINDER_FILE_FLAG_INVISIBLE = (1 << 14),
		FINDER_FILE_FLAG_BUNDLE = (1 << 13),
		FINDER_FILE_FLAG_NAME_LOCKED = (1 << 12),
		FINDER_FILE_FLAG_STATIONARY = (1 << 11),
		FINDER_FILE_FLAG_CUSTOM_ICON = (1 << 10),
		FINDER_FILE_FLAG_INITED = (1 << 8),
		FINDER_FILE_FLAG_NO_INITS = (1 << 7),
		FINDER_FILE_FLAG_SHARED = (1 << 6),
		FINDER_FILE_FLAG_COLOR_BIT2 = (1 << 3),
		FINDER_FILE_FLAG_COLOR_BIT1 = (1 << 2),
		FINDER_FILE_FLAG_COLOR_BIT0 = (1 << 1),
	};

	struct MacFileProperties
	{
		MacFileProperties();

		char m_fileType[4];
		char m_fileCreator[4];
		int16_t m_xPos;
		int16_t m_yPos;
		uint16_t m_finderFlags;
		uint8_t m_protected;
		int64_t m_createdTimeMacEpoch;
		int64_t m_modifiedTimeMacEpoch;
	};

	struct MacFilePropertiesSerialized
	{
		static const unsigned int kOffsetFileType = 0;
		static const unsigned int kOffsetFileCreator = 4;
		static const unsigned int kOffsetXPos = 8;
		static const unsigned int kOffsetYPos = 10;
		static const unsigned int kOffsetFinderFlags = 12;
		static const unsigned int kOffsetProtected = 14;
		static const unsigned int kOffsetCreatedDate = 15;
		static const unsigned int kOffsetModifiedDate = 23;

		static const unsigned int kSize = 31;

		uint8_t m_data[kSize];

		void Deserialize(MacFileProperties &props) const;
		void Serialize(const MacFileProperties &props);

		bool WriteAsPackage(GpIOStream &stream, const CombinedTimestamp &ts) const;
		bool WriteIsolated(GpIOStream &stream, const CombinedTimestamp &ts) const;
		bool ReadFromPackage(GpIOStream &stream);

		static const char *GetPackagedName();
	};

	struct MacFileInfo
	{
		MacFileInfo();

		PascalStr<64> m_fileName;
		uint16_t m_commentSize;
		uint32_t m_dataForkSize;
		uint32_t m_resourceForkSize;

		MacFileProperties m_properties;
	};
}

namespace PortabilityLayer
{
	inline MacFileProperties::MacFileProperties()
		: m_xPos(0)
		, m_yPos(0)
		, m_finderFlags(0)
		, m_protected(0)
		, m_createdTimeMacEpoch(0)
		, m_modifiedTimeMacEpoch(0)
	{
		m_fileType[0] = m_fileType[1] = m_fileType[2] = m_fileType[3] = '\0';
		m_fileCreator[0] = m_fileCreator[1] = m_fileCreator[2] = m_fileCreator[3] = '\0';
	}

	inline MacFileInfo::MacFileInfo()
		: m_dataForkSize(0)
		, m_resourceForkSize(0)
		, m_commentSize(0)
	{
	}
}
