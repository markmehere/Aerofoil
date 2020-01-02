#pragma once

#ifndef __PL_CFILESTREAM_H__
#define __PL_CFILESTREAM_H__

#include <stdio.h>

#include "CoreDefs.h"
#include "IOStream.h"

namespace PortabilityLayer
{
	class CFileStream final : public IOStream
	{
	public:
		explicit CFileStream(FILE *f);
		CFileStream(FILE *f, bool isReadOnly, bool isWriteOnly, bool isSeekable);

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

	private:
		CFileStream(const CFileStream &other) GP_DELETED;

		FILE *m_file;
		bool m_readOnly;
		bool m_writeOnly;
		bool m_seekable;
	};
}

#endif
