#pragma once
#ifndef __PL_RES_TYPE_ID_H__
#define __PL_RES_TYPE_ID_H__

#include <stdint.h>

namespace PortabilityLayer
{
	class ResTypeID
	{
	public:
		ResTypeID();
		ResTypeID(int32_t i);
		ResTypeID(const ResTypeID &other);
		explicit ResTypeID(const char *chars);

		ResTypeID &operator=(const ResTypeID &other);
		bool operator==(const ResTypeID &other) const;
		bool operator!=(const ResTypeID &other) const;

		void ExportAsChars(char *chars) const;
		int32_t ExportAsInt32() const;

	private:
		char m_id[4];
	};
}

#include "ResTypeIDCodec.h"
#include <string.h>

namespace PortabilityLayer
{
	inline ResTypeID::ResTypeID()
	{
		m_id[0] = m_id[1] = m_id[2] = m_id[3] = 0;
	}

	inline ResTypeID::ResTypeID(int32_t i)
	{
		ResTypeIDCodec::Encode(i, m_id);
	}

	inline ResTypeID::ResTypeID(const ResTypeID &other)
	{
		memcpy(m_id, other.m_id, 4);
	}

	inline ResTypeID::ResTypeID(const char *chars)
	{
		memcpy(m_id, chars, 4);
	}

	inline ResTypeID &ResTypeID::operator=(const ResTypeID &other)
	{
		memcpy(m_id, other.m_id, 4);
		return *this;
	}

	inline bool ResTypeID::operator==(const ResTypeID &other) const
	{
		return memcmp(m_id, other.m_id, 4) == 0;
	}

	inline bool ResTypeID::operator!=(const ResTypeID &other) const
	{
		return memcmp(m_id, other.m_id, 4) != 0;
	}

	inline void ResTypeID::ExportAsChars(char *chars) const
	{
		memcpy(chars, m_id, 4);
	}

	inline int32_t ResTypeID::ExportAsInt32() const
	{
		return ResTypeIDCodec::Decode(m_id);
	}
}

#endif
