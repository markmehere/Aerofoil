#pragma once

#ifndef __PL_CFILESTREAM_H__
#define __PL_CFILESTREAM_H__

#include <stdio.h>

#include "CoreDefs.h"
#include "GpIOStream.h"

namespace PortabilityLayer
{
	class CFileStream final : public GpIOStream
	{
	public:
		explicit CFileStream(FILE *f);
		CFileStream(FILE *f, bool isReadOnly, bool isWriteOnly, bool isSeekable);

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
		CFileStream(const CFileStream &other) GP_DELETED;

		FILE *m_file;
		bool m_readOnly;
		bool m_writeOnly;
		bool m_seekable;
	};
}

#endif
