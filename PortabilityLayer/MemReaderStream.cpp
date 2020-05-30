#include "MemReaderStream.h"

#include <string.h>

namespace PortabilityLayer
{
	MemReaderStream::MemReaderStream(const void *memStream, size_t size)
		: m_bytes(static_cast<const uint8_t*>(memStream))
		, m_size(size)
		, m_loc(0)
	{
	}

	size_t MemReaderStream::Read(void *bytesOut, size_t size)
	{
		size_t available = m_size - m_loc;
		if (size > available)
			size = available;

		memcpy(bytesOut, m_bytes + m_loc, size);
		m_loc += size;

		return size;
	}

	size_t MemReaderStream::Write(const void *bytes, size_t size)
	{
		return 0;
	}

	bool IsSeekable()
	{
		return true;
	}

	bool MemReaderStream::IsSeekable() const
	{
		return true;
	}

	bool MemReaderStream::IsReadOnly() const
	{
		return true;
	}

	bool MemReaderStream::IsWriteOnly() const
	{
		return false;
	}

	bool MemReaderStream::SeekStart(UFilePos_t loc)
	{
		if (loc > m_size)
			m_loc = m_size;
		else
			m_loc = static_cast<size_t>(loc);

		return true;
	}

	bool MemReaderStream::SeekCurrent(FilePos_t loc)
	{
		if (loc < 0)
		{
			if (static_cast<FilePos_t>(m_loc) + loc < 0)
				m_loc = 0;
			else
				m_loc = static_cast<size_t>(static_cast<FilePos_t>(m_loc) + loc);
		}
		else
		{
			const size_t available = m_size - m_loc;
			if (static_cast<UFilePos_t>(loc) > available)
				m_loc = m_size;
			else
				m_loc = static_cast<size_t>(static_cast<FilePos_t>(m_loc) + loc);
		}

		return true;
	}

	bool MemReaderStream::SeekEnd(UFilePos_t loc)
	{
		if (m_size < loc)
			m_loc = 0;
		else
			m_loc = m_size - static_cast<size_t>(loc);

		return true;
	}

	bool MemReaderStream::Truncate(UFilePos_t loc)
	{
		return false;
	}

	UFilePos_t MemReaderStream::Size() const
	{
		return m_size;
	}

	UFilePos_t MemReaderStream::Tell() const
	{
		return static_cast<UFilePos_t>(m_loc);
	}

	void MemReaderStream::Close()
	{
	}

	void MemReaderStream::Flush()
	{
	}
}
