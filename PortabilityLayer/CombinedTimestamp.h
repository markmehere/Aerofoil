#pragma once

#include <stdint.h>

namespace PortabilityLayer
{
	struct CombinedTimestamp
	{
		uint8_t m_utcTimestamp[8];

		uint8_t m_localYear[4];
		uint8_t m_localMonth;
		uint8_t m_localDay;

		uint8_t m_localHour;
		uint8_t m_localMinute;
		uint8_t m_localSecond;

		uint8_t m_padding[3];

		int64_t GetUTCTime() const;
		void SetUTCTime(int64_t timestamp);

		int32_t GetLocalYear() const;
		void SetLocalYear(int32_t year);
	};

	inline int64_t CombinedTimestamp::GetUTCTime() const
	{
		int64_t result = 0;
		for (int i = 0; i < 8; i++)
			result |= static_cast<int64_t>(m_utcTimestamp[i]) << (i * 8);

		return result;
	}

	void CombinedTimestamp::SetUTCTime(int64_t timestamp)
	{
		for (int i = 0; i < 8; i++)
			m_utcTimestamp[i] = static_cast<uint8_t>((timestamp >> (i * 8)) & 0xff);
	}

	inline int32_t CombinedTimestamp::GetLocalYear() const
	{
		int32_t result = 0;
		for (int i = 0; i < 4; i++)
			result |= static_cast<int32_t>(m_localYear[i]) << (i * 8);

		return result;
	}

	void CombinedTimestamp::SetLocalYear(int32_t timestamp)
	{
		for (int i = 0; i < 4; i++)
			m_localYear[i] = static_cast<uint8_t>((timestamp >> (i * 8)) & 0xff);
	}
}
