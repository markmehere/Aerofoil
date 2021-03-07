#include "InflateStream.h"
#include "DeflateCodec.h"

#include <stdlib.h>
#include <new>

namespace PortabilityLayer
{
	class InflateStreamImpl final : public GpIOStream
	{
	public:
		InflateStreamImpl(GpIOStream *stream, GpUFilePos_t start, size_t compressedSize, size_t decompressedSize, InflateContext *inflateContext);
		InflateStreamImpl();

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
		void Close() override;
		void Flush() override;

	private:
		GpIOStream *m_stream;
		InflateContext *m_inflateContext;
		bool m_contextBroken;

		GpUFilePos_t m_start;
		GpUFilePos_t m_compressedSize;
		GpUFilePos_t m_compressedPos;
		size_t m_decompressedSize;
		size_t m_decompressedPos;

		uint8_t m_compressedInputBytes[1024];
		size_t m_compressedInputReadOffset;
		size_t m_compressedInputSize;
	};


	InflateStreamImpl::InflateStreamImpl(GpIOStream *stream, GpUFilePos_t start, size_t compressedSize, size_t decompressedSize, InflateContext *inflateContext)
		: m_stream(stream)
		, m_start(start)
		, m_compressedPos(start)
		, m_compressedSize(compressedSize)
		, m_decompressedSize(decompressedSize)
		, m_decompressedPos(0)
		, m_inflateContext(inflateContext)
		, m_compressedInputReadOffset(0)
		, m_compressedInputSize(0)
		, m_contextBroken(false)
	{
	}

	InflateStreamImpl::InflateStreamImpl()
	{
	}

	size_t InflateStreamImpl::Read(void *bytesOut, size_t size)
	{
		if (m_contextBroken)
			return 0;

		size_t sizeAvailable = m_decompressedSize - m_decompressedPos;
		if (size > sizeAvailable)
			size = sizeAvailable;

		size_t totalConsumed = 0;

		while (size > 0)
		{
			size_t sizeConsumed = 0;
			if (!m_inflateContext->Read(bytesOut, size, sizeConsumed))
			{
				m_contextBroken = true;
				return 0;
			}

			if (bytesOut)
				bytesOut = static_cast<uint8_t*>(bytesOut) + sizeConsumed;
			size -= sizeConsumed;
			m_decompressedPos += sizeConsumed;
			totalConsumed += sizeConsumed;

			if (sizeConsumed == 0)
			{
				if (m_compressedInputSize == m_compressedInputReadOffset)
				{
					if (m_compressedPos != m_stream->Tell())
					{
						if (!m_stream->SeekStart(m_compressedPos))
							return 0;
					}

					GpUFilePos_t compressedAvailable = m_compressedSize - (m_compressedPos - m_start);
					if (compressedAvailable == 0)
						return totalConsumed;

					size_t compressedToRead = sizeof(m_compressedInputBytes);

					if (compressedToRead > compressedAvailable)
						compressedToRead = compressedAvailable;

					if (!m_stream->ReadExact(m_compressedInputBytes, compressedToRead))
						return 0;

					m_compressedInputReadOffset = 0;
					m_compressedInputSize = compressedToRead;
					m_compressedPos += compressedToRead;
				}

				size_t compressedInputAvailable = m_compressedInputSize - m_compressedInputReadOffset;
				size_t compressedInputConsumed = 0;

				if (!m_inflateContext->Append(m_compressedInputBytes + m_compressedInputReadOffset, compressedInputAvailable, compressedInputConsumed))
				{
					m_contextBroken = true;
					return 0;
				}

				if (compressedInputConsumed == 0)
					return 0;	// This should never happen

				m_compressedInputReadOffset += compressedInputConsumed;
			}
		}

		return totalConsumed;
	}

	size_t InflateStreamImpl::Write(const void *bytes, size_t size)
	{
		return 0;
	}

	bool InflateStreamImpl::IsSeekable() const
	{
		return true;
	}

	bool InflateStreamImpl::IsReadOnly() const
	{
		return true;
	}

	bool InflateStreamImpl::IsWriteOnly() const
	{
		return false;
	}

	bool InflateStreamImpl::SeekStart(GpUFilePos_t loc)
	{
		if (m_contextBroken)
			return false;

		if (loc > m_decompressedSize)
			return false;

		if (loc < m_decompressedPos)
		{
			if (!m_inflateContext->Reset())
			{
				m_contextBroken = true;
				return false;
			}

			m_decompressedPos = 0;
			m_compressedInputReadOffset = 0;
			m_compressedInputSize = 0;
			m_compressedPos = m_start;
		}

		size_t skipAhead = static_cast<size_t>(loc) - m_decompressedPos;

		return this->Read(nullptr, skipAhead) == skipAhead;
	}

	bool InflateStreamImpl::SeekCurrent(GpFilePos_t loc)
	{
		if (m_contextBroken)
			return false;

		if (loc < 0)
		{
			GpUFilePos_t negativePos = static_cast<GpUFilePos_t>(-loc);
			if (negativePos > m_decompressedPos)
				return false;

			return SeekStart(m_decompressedPos - negativePos);
		}
		else if (loc > 0)
		{
			GpUFilePos_t positivePos = static_cast<GpUFilePos_t>(-loc);
			if (positivePos > m_decompressedSize - m_decompressedPos)
				return false;

			return this->Read(nullptr, static_cast<size_t>(positivePos));
		}
		else
			return true;
	}

	bool InflateStreamImpl::SeekEnd(GpUFilePos_t loc)
	{
		if (loc > m_decompressedSize)
			return false;

		return this->SeekStart(m_decompressedSize - loc);
	}

	GpUFilePos_t InflateStreamImpl::Size() const
	{
		return m_decompressedSize;
	}

	GpUFilePos_t InflateStreamImpl::Tell() const
	{
		return m_decompressedPos;
	}

	void InflateStreamImpl::Close()
	{
		this->~InflateStreamImpl();
		free(this);
	}

	void InflateStreamImpl::Flush()
	{
	}

	GpIOStream *InflateStream::Create(GpIOStream *stream, GpUFilePos_t start, size_t compressedSize, size_t decompressedSize)
	{
		InflateContext *inflateContext = InflateContext::Create();
		if (!inflateContext)
			return nullptr;

		void *storage = malloc(sizeof(InflateStreamImpl));
		if (!storage)
		{
			inflateContext->Destroy();
			return nullptr;
		}

		return new (storage) InflateStreamImpl(stream, start, compressedSize, decompressedSize, inflateContext);
	}
}
