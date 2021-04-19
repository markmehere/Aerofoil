#pragma once

#include "GpCoreDefs.h"
#include "GpWindows.h"
#include "GpIOStream.h"

class GpFileStream_Win32 final : public GpIOStream
{
public:
	explicit GpFileStream_Win32(HANDLE handle, bool readable, bool writeable, bool seekable);

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

#if GP_ASYNCIFY_PARANOID
	void Close();
#endif

private:
	HANDLE m_handle;
	bool m_readable;
	bool m_writeable;
	bool m_seekable;
};
