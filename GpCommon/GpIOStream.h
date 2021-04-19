#pragma once

#include <stddef.h>

#include "GpFilePos.h"
#include "CoreDefs.h"

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
	virtual GpUFilePos_t Size() const = 0;
	virtual GpUFilePos_t Tell() const = 0;
	virtual void GP_ASYNCIFY_PARANOID_NAMED(Close)() = 0;
	virtual void Flush() = 0;

	bool ReadExact(void *bytesOut, size_t size);
	bool WriteExact(const void *bytesOut, size_t size);

#if GP_ASYNCIFY_PARANOID
	void Close();
#endif
};

inline bool GpIOStream::ReadExact(void *bytesOut, size_t size)
{
	const size_t nRead = this->Read(bytesOut, size);
	return nRead == size;
}

inline bool GpIOStream::WriteExact(const void *bytes, size_t size)
{
	const size_t nWritten = this->Write(bytes, size);
	return nWritten == size;
}
