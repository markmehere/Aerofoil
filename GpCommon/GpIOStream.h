#pragma once

#include "GpFilePos.h"

class GpIOStream
{
public:
	virtual size_t Read(void *bytesOut, size_t size) = 0;
	virtual size_t Write(const void *bytes, size_t size) = 0;
	virtual bool IsSeekable() const = 0;
	virtual bool IsReadOnly() const = 0;
	virtual bool IsWriteOnly() const = 0;
	virtual bool SeekStart(GpUFilePos_t loc) = 0;
	virtual bool SeekCurrent(GpFilePos_t loc) = 0;
	virtual bool SeekEnd(GpUFilePos_t loc) = 0;
	virtual bool Truncate(GpUFilePos_t loc) = 0;
	virtual GpUFilePos_t Size() const = 0;
	virtual GpUFilePos_t Tell() const = 0;
	virtual void Close() = 0;
	virtual void Flush() = 0;
};
