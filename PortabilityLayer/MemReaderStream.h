#pragma once
#ifndef __PL_MEM_READER_STREAM_H__
#define __PL_MEM_READER_STREAM_H__

#include "CoreDefs.h"
#include "IOStream.h"

namespace PortabilityLayer
{
	class MemReaderStream final : public IOStream
	{
	public:
		MemReaderStream(const void *memStream, size_t size);

		size_t Read(void *bytesOut, size_t size) override;
		size_t Write(const void *bytes, size_t size) override;
		bool IsSeekable() const override;
		bool IsReadOnly() const override;
		bool IsWriteOnly() const override;
		bool SeekStart(UFilePos_t loc) override;
		bool SeekCurrent(FilePos_t loc) override;
		bool SeekEnd(UFilePos_t loc) override;
		bool Truncate(UFilePos_t loc) override;
		UFilePos_t Size() const override;
		UFilePos_t Tell() const override;
		void Close() override;
		void Flush() override;

	private:
		MemReaderStream() GP_DELETED;

		const uint8_t *m_bytes;
		size_t m_size;
		size_t m_loc;
	};
}

#endif
