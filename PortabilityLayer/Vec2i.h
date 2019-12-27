#pragma once

#include <stdint.h>

namespace PortabilityLayer
{
	struct Vec2i
	{
		int32_t m_x;
		int32_t m_y;

		Vec2i();
		Vec2i(const Vec2i &other);
		Vec2i(int32_t x, int32_t y);

		Vec2i operator+(const Vec2i &other) const;
		Vec2i operator-(const Vec2i &other) const;

		Vec2i &operator+=(const Vec2i &other);
		Vec2i &operator-=(const Vec2i &other);

		bool operator==(const Vec2i &other) const;
		bool operator!=(const Vec2i &other) const;
	};

	inline Vec2i::Vec2i()
		: m_x(0)
		, m_y(0)
	{
	}

	inline Vec2i::Vec2i(const Vec2i &other)
		: m_x(other.m_x)
		, m_y(other.m_y)
	{
	}

	inline Vec2i::Vec2i(int32_t x, int32_t y)
		: m_x(x)
		, m_y(y)
	{
	}

	inline Vec2i Vec2i::operator+(const Vec2i &other) const
	{
		return Vec2i(m_x + other.m_x, m_y + other.m_y);
	}

	inline Vec2i Vec2i::operator-(const Vec2i &other) const
	{
		return Vec2i(m_x - other.m_x, m_y - other.m_y);
	}

	inline Vec2i &Vec2i::operator+=(const Vec2i &other)
	{
		m_x += other.m_x;
		m_y += other.m_y;
		return *this;
	}

	inline Vec2i &Vec2i::operator-=(const Vec2i &other)
	{
		m_x -= other.m_x;
		m_y -= other.m_y;
		return *this;
	}

	inline bool Vec2i::operator==(const Vec2i &other) const
	{
		return m_x == other.m_x && m_y == other.m_y;
	}

	inline bool Vec2i::operator!=(const Vec2i &other) const
	{
		return !((*this) == other);
	}
}
