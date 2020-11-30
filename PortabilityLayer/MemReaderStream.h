#pragma once
#ifndef __PL_MEM_READER_STREAM_H__
#define __PL_MEM_READER_STREAM_H__

#include "CoreDefs.h"
#include "GpIOStream.h"

namespace PortabilityLayer
{
	class MemReaderStream : public GpIOStream
	{
	public:
		MemReaderStream(const void *memStream, size_t size);
		virtual ~MemReaderStream();

		size_t Read(void *bytesOut, size_t size) override;
		size_t Write(const void *bytes, size_t size) override;
		bool IsSeekable() const override;
		bool IsReadOnly() const override;
		bool IsWriteOnly() const override;
		bool SeekStart(GpUFilePos_t loc) override;
		bool SeekCurrent(GpFilePos_t loc) override;
		bool SeekEnd(GpUFilePos_t loc) override;
		bool Truncate(GpUFilePos_t loc) override;
		GpUFilePos_t Size() const override;
		GpUFilePos_t Tell() const override;
		void Close() override;
		void Flush() override;

	private:
		MemReaderStream() GP_DELETED;

		const uint8_t *m_bytes;
		size_t m_size;
		size_t m_loc;
	};

	class MemBufferReaderStream final : public MemReaderStream
	{
	public:
		~MemBufferReaderStream() override;

		static MemBufferReaderStream *Create(void *buffer, size_t size);

		void Close() override;

	private:
		MemBufferReaderStream() GP_DELETED;
		MemBufferReaderStream(void *buffer, size_t size);

		void *m_buffer;
	};
}

#endif
