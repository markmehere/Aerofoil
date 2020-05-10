#pragma once

#include "IFileReader.h"

class IDecompressor;

class DecompressorProxyReader final : public IFileReader
{
public:
	explicit DecompressorProxyReader(IDecompressor *decompressor);

	size_t Read(void *buffer, size_t sz) override;
	size_t FileSize() const override;
	bool SeekStart(FilePos_t pos) override;
	bool SeekCurrent(FilePos_t pos) override;
	bool SeekEnd(FilePos_t pos) override;
	FilePos_t GetPosition() const override;

private:
	FilePos_t m_currentPosition;
	IDecompressor *m_decompressor;
};
