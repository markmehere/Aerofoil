#pragma once

#include "GpCoreDefs.h"
#include "GpWindows.h"
#include "IOStream.h"

class GpFileStream_Win32 final : public PortabilityLayer::IOStream
{
public:
	explicit GpFileStream_Win32(HANDLE handle, bool readable, bool writeable, bool seekable);

	size_t Read(void *bytesOut, size_t size) override;
	size_t Write(const void *bytes, size_t size) override;
	bool IsSeekable() const override;
	bool IsReadOnly() const override;
	bool IsWriteOnly() const override;
	bool SeekStart(PortabilityLayer::UFilePos_t loc) override;
	bool SeekCurrent(PortabilityLayer::FilePos_t loc) override;
	bool SeekEnd(PortabilityLayer::UFilePos_t loc) override;
	bool Truncate(PortabilityLayer::UFilePos_t loc) override;
	PortabilityLayer::UFilePos_t Size() const override;
	PortabilityLayer::UFilePos_t Tell() const override;
	void Close() override;

private:
	HANDLE m_handle;
	bool m_readable;
	bool m_writeable;
	bool m_seekable;
};
