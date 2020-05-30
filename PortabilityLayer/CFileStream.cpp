#include "CFileStream.h"

namespace PortabilityLayer
{
	CFileStream::CFileStream(FILE *f)
		: m_file(f)
		, m_readOnly(false)
		, m_writeOnly(false)
		, m_seekable(true)
	{
	}

	CFileStream::CFileStream(FILE *f, bool isReadOnly, bool isWriteOnly, bool isSeekable)
		: m_file(f)
		, m_readOnly(isReadOnly)
		, m_writeOnly(isWriteOnly)
		, m_seekable(isSeekable)
	{
	}

	size_t CFileStream::Read(void *bytesOut, size_t size)
	{
		if (!m_file || m_writeOnly)
			return 0;

		return fread(bytesOut, 1, size, m_file);
	}

	size_t CFileStream::Write(const void *bytes, size_t size)
	{
		if (!m_file || m_readOnly)
			return 0;

		return fwrite(bytes, 1, size, m_file);
	}

	bool CFileStream::IsSeekable() const
	{
		return m_seekable;
	}

	bool CFileStream::IsReadOnly() const
	{
		return m_readOnly;
	}

	bool CFileStream::IsWriteOnly() const
	{
		return m_writeOnly;
	}

	bool CFileStream::SeekStart(UFilePos_t loc)
	{
		if (!m_file)
			return false;

		return fseek(m_file, static_cast<long>(loc), SEEK_SET) == 0;
	}

	bool CFileStream::SeekCurrent(FilePos_t loc)
	{
		if (!m_file)
			return false;

		return fseek(m_file, static_cast<long>(loc), SEEK_CUR) == 0;;
	}

	bool CFileStream::SeekEnd(UFilePos_t loc)
	{
		if (!m_file)
			return false;

		return fseek(m_file, static_cast<long>(loc), SEEK_END) == 0;
	}

	bool CFileStream::Truncate(UFilePos_t loc)
	{
		return false;
	}

	UFilePos_t CFileStream::Tell() const
	{
		if (!m_file)
			return 0;

		return static_cast<UFilePos_t>(ftell(m_file));
	}

	void CFileStream::Close()
	{
		if (m_file)
		{
			fclose(m_file);
			m_file = nullptr;
		}
	}

	void CFileStream::Flush()
	{
		if (m_file)
			fflush(m_file);
	}


	UFilePos_t CFileStream::Size() const
	{
		if (!m_file || !m_seekable)
			return 0;

		long oldPos = ftell(m_file);
		fseek(m_file, 0, SEEK_END);
		const UFilePos_t endPos = static_cast<UFilePos_t>(ftell(m_file));
		fseek(m_file, oldPos, SEEK_SET);

		return endPos;
	}
}
