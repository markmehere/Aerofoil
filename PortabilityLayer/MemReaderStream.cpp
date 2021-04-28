#include "MemReaderStream.h"
#include "MemoryManager.h"

#include <string.h>

namespace PortabilityLayer
{
	MemReaderStream::MemReaderStream(const void *memStream, size_t size)
		: m_bytes(static_cast<const uint8_t*>(memStream))
		, m_size(size)
		, m_loc(0)
	{
	}

	MemReaderStream::~MemReaderStream()
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

	bool MemReaderStream::SeekStart(GpUFilePos_t loc)
	{
		if (loc > m_size)
			m_loc = m_size;
		else
			m_loc = static_cast<size_t>(loc);

		return true;
	}

	bool MemReaderStream::SeekCurrent(GpFilePos_t loc)
	{
		if (loc < 0)
		{
			if (static_cast<GpFilePos_t>(m_loc) + loc < 0)
				m_loc = 0;
			else
				m_loc = static_cast<size_t>(static_cast<GpFilePos_t>(m_loc) + loc);
		}
		else
		{
			const size_t available = m_size - m_loc;
			if (static_cast<GpUFilePos_t>(loc) > available)
				m_loc = m_size;
			else
				m_loc = static_cast<size_t>(static_cast<GpFilePos_t>(m_loc) + loc);
		}

		return true;
	}

	bool MemReaderStream::SeekEnd(GpUFilePos_t loc)
	{
		if (m_size < loc)
			m_loc = 0;
		else
			m_loc = m_size - static_cast<size_t>(loc);

		return true;
	}

	GpUFilePos_t MemReaderStream::Size() const
	{
		return m_size;
	}

	GpUFilePos_t MemReaderStream::Tell() const
	{
		return static_cast<GpUFilePos_t>(m_loc);
	}

	void MemReaderStream::GP_ASYNCIFY_PARANOID_NAMED(Close)()
	{
	}

	void MemReaderStream::Flush()
	{
	}


	MemBufferReaderStream::~MemBufferReaderStream()
	{
		if (m_buffer)
			MemoryManager::GetInstance()->Release(m_buffer);
	}

	MemBufferReaderStream *MemBufferReaderStream::Create(void *buffer, size_t size)
	{
		void *storage = MemoryManager::GetInstance()->Alloc(sizeof(MemBufferReaderStream));
		if (!storage)
			return nullptr;

		return new (storage) MemBufferReaderStream(buffer, size);
	}

	void MemBufferReaderStream::GP_ASYNCIFY_PARANOID_NAMED(Close)()
	{
		this->~MemBufferReaderStream();
		MemoryManager::GetInstance()->Release(this);
	}

	MemBufferReaderStream::MemBufferReaderStream(void *buffer, size_t size)
		: MemReaderStream(buffer, size)
		, m_buffer(buffer)
	{
	}
}
