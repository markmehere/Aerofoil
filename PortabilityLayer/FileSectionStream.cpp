#include "FileSectionStream.h"
#include "PLCore.h"

#include <stdlib.h>
#include <new>

namespace PortabilityLayer
{
	class FileSectionStreamImpl final : public GpIOStream
	{
	public:
		FileSectionStreamImpl(GpIOStream *stream, GpUFilePos_t start, GpUFilePos_t size);
		~FileSectionStreamImpl();

		size_t Read(void *bytesOut, size_t size) override;
		size_t Write(const void *bytes, size_t size) override;
		bool IsSeekable() const override;
		bool IsReadOnly() const override;
		bool IsWriteOnly() const override;
		bool SeekStart(GpUFilePos_t loc) override;
		bool SeekCurrent(GpFilePos_t loc) override;
		bool SeekEnd(GpUFilePos_t loc) override;
		GpUFilePos_t Size() const override;
		GpUFilePos_t Tell() const override;
		void GP_ASYNCIFY_PARANOID_NAMED(Close)() override;
		void Flush() override;

	private:
		GpIOStream *m_stream;
		GpUFilePos_t m_start;
		GpUFilePos_t m_size;

		GpUFilePos_t m_expectedPosition;
		bool m_isSeekable;
	};

	FileSectionStreamImpl::FileSectionStreamImpl(GpIOStream *stream, GpUFilePos_t start, GpUFilePos_t size)
		: m_stream(stream)
		, m_start(start)
		, m_size(size)
		, m_expectedPosition(start)
		, m_isSeekable(stream->IsSeekable())
	{
	}

	FileSectionStreamImpl::~FileSectionStreamImpl()
	{
	}

	size_t FileSectionStreamImpl::Read(void *bytesOut, size_t size)
	{
		if (m_stream->Tell() != m_expectedPosition)
		{
			if (!m_stream->SeekStart(m_expectedPosition))
				return 0;
		}

		GpUFilePos_t localPos = m_expectedPosition - m_start;
		GpUFilePos_t availableBytes = m_size - localPos;

		if (size > availableBytes)
			size = static_cast<size_t>(availableBytes);

		const size_t actuallyRead = m_stream->Read(bytesOut, size);
		m_expectedPosition += actuallyRead;

		return actuallyRead;
	}

	size_t FileSectionStreamImpl::Write(const void *bytes, size_t size)
	{
		return 0;
	}

	bool FileSectionStreamImpl::IsSeekable() const
	{
		return m_isSeekable;
	}

	bool FileSectionStreamImpl::IsReadOnly() const
	{
		return true;
	}

	bool FileSectionStreamImpl::IsWriteOnly() const
	{
		return false;
	}

	bool FileSectionStreamImpl::SeekStart(GpUFilePos_t loc)
	{
		if (loc == m_expectedPosition - m_start)
			return true;

		if (!m_isSeekable)
			return false;

		if (loc >= m_size)
			return false;
		else
		{
			m_expectedPosition = m_start + loc;
			return true;
		}
	}

	bool FileSectionStreamImpl::SeekCurrent(GpFilePos_t loc)
	{
		GpUFilePos_t localPos = m_expectedPosition - m_start;

		if (loc < 0)
		{
			GpUFilePos_t negativePos = static_cast<GpUFilePos_t>(-loc);
			if (negativePos > localPos)
				return false;

			m_expectedPosition -= negativePos;
			return true;
		}
		else if (loc > 0)
		{
			GpUFilePos_t positivePos = static_cast<GpUFilePos_t>(loc);
			if (m_size - localPos < positivePos)
				return false;

			m_expectedPosition += positivePos;
			return true;
		}
		else
			return true;
	}

	bool FileSectionStreamImpl::SeekEnd(GpUFilePos_t loc)
	{
		if (loc > m_size)
			return false;

		m_expectedPosition = m_start + (m_size - loc);
		return true;
	}

	GpUFilePos_t FileSectionStreamImpl::Size() const
	{
		return m_size;
	}

	GpUFilePos_t FileSectionStreamImpl::Tell() const
	{
		return m_expectedPosition - m_start;
	}

	void FileSectionStreamImpl::GP_ASYNCIFY_PARANOID_NAMED(Close)()
	{
		this->~FileSectionStreamImpl();
		DisposePtr(this);
	}

	void FileSectionStreamImpl::Flush()
	{
		m_stream->Flush();
	}

	GpIOStream *FileSectionStream::Create(GpIOStream *stream, GpUFilePos_t start, GpUFilePos_t size)
	{
		void *storage = NewPtr(sizeof(FileSectionStreamImpl));

		if (!storage)
			return nullptr;

		return new (storage) FileSectionStreamImpl(stream, start, size);
	}
}
