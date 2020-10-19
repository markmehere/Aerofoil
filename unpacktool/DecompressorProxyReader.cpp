#include "DecompressorProxyReader.h"

#include "IDecompressor.h"

DecompressorProxyReader::DecompressorProxyReader(IDecompressor *decompressor)
	: m_decompressor(decompressor)
	, m_currentPosition(0)
{
}

size_t DecompressorProxyReader::Read(void *buffer, size_t sz)
{
	if (!m_decompressor->ReadBytes(buffer, sz))
		return 0;

	m_currentPosition += sz;

	return sz;
}

size_t DecompressorProxyReader::FileSize() const
{
	return 0;
}

bool DecompressorProxyReader::SeekStart(FilePos_t pos)
{
	return pos == m_currentPosition;
}

bool DecompressorProxyReader::SeekCurrent(FilePos_t pos)
{
	return pos == 0;
}

bool DecompressorProxyReader::SeekEnd(FilePos_t pos)
{
	return false;
}

IFileReader::FilePos_t DecompressorProxyReader::GetPosition() const
{
	return m_currentPosition;
}

