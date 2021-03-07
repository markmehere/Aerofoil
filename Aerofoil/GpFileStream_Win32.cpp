#include "GpFileStream_Win32.h"

GpFileStream_Win32::GpFileStream_Win32(HANDLE handle, bool readable, bool writeable, bool seekable)
	: m_handle(handle)
	, m_readable(readable)
	, m_writeable(writeable)
	, m_seekable(seekable)
{
}

size_t GpFileStream_Win32::Read(void *bytesOut, size_t size)
{
	if (!m_readable)
		return 0;

	size_t totalRead = 0;
	while (size)
	{
		const DWORD chunkSizeToRead = (size > MAXDWORD) ? MAXDWORD : size;
		DWORD numRead = 0;

		BOOL readSucceeded = ReadFile(m_handle, bytesOut, chunkSizeToRead, &numRead, nullptr);
		if (!readSucceeded)
			return totalRead;

		totalRead += static_cast<size_t>(numRead);
		size -= static_cast<size_t>(numRead);
		bytesOut = static_cast<void*>(static_cast<uint8_t*>(bytesOut) + numRead);

		if (numRead != chunkSizeToRead)
			return totalRead;
	}

	return totalRead;
}

size_t GpFileStream_Win32::Write(const void *bytes, size_t size)
{
	if (!m_writeable)
		return 0;

	size_t totalWritten = 0;
	while (size)
	{
		const DWORD chunkSizeToWrite = (size > MAXDWORD) ? MAXDWORD : size;
		DWORD numWritten = 0;

		BOOL writeSucceeded = WriteFile(m_handle, bytes, chunkSizeToWrite, &numWritten, nullptr);
		if (!writeSucceeded)
		{
			DWORD lastError = GetLastError();
			return totalWritten;
		}

		totalWritten += static_cast<size_t>(numWritten);
		size -= static_cast<size_t>(numWritten);
		bytes = static_cast<const void*>(static_cast<const uint8_t*>(bytes) + numWritten);

		if (numWritten != chunkSizeToWrite)
			return totalWritten;
	}

	return totalWritten;
}

bool GpFileStream_Win32::IsSeekable() const
{
	return m_seekable;
}

bool GpFileStream_Win32::IsReadOnly() const
{
	return !m_writeable;
}

bool GpFileStream_Win32::IsWriteOnly() const
{
	return !m_readable;
}

bool GpFileStream_Win32::SeekStart(GpUFilePos_t loc)
{
	LARGE_INTEGER li;
	li.QuadPart = static_cast<LONGLONG>(loc);
	return SetFilePointerEx(m_handle, li, nullptr, FILE_BEGIN) != 0;
}

bool GpFileStream_Win32::SeekCurrent(GpFilePos_t loc)
{
	LARGE_INTEGER li;
	li.QuadPart = static_cast<LONGLONG>(loc);
	return SetFilePointerEx(m_handle, li, nullptr, FILE_CURRENT) != 0;
}

bool GpFileStream_Win32::SeekEnd(GpUFilePos_t loc)
{
	LARGE_INTEGER li;
	li.QuadPart = -static_cast<LONGLONG>(loc);
	return SetFilePointerEx(m_handle, li, nullptr, FILE_END) != 0;
}

GpUFilePos_t GpFileStream_Win32::Size() const
{
	LARGE_INTEGER fsize;
	if (!GetFileSizeEx(m_handle, &fsize))
		return 0;

	return static_cast<GpUFilePos_t>(fsize.QuadPart);
}

GpUFilePos_t GpFileStream_Win32::Tell() const
{
	LARGE_INTEGER zero;
	zero.QuadPart = 0;

	LARGE_INTEGER fpos;
	if (!SetFilePointerEx(m_handle, zero, &fpos, FILE_CURRENT))
		return 0;

	return static_cast<GpUFilePos_t>(fpos.QuadPart);
}

void GpFileStream_Win32::Close()
{
	CloseHandle(m_handle);
}

void GpFileStream_Win32::Flush()
{
	FlushFileBuffers(m_handle);
}
