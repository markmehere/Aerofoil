#include "CompactProLZHRLEDecompressor.h"
#include "CompactProRLEDecompressor.h"
#include "CompactProLZHDecompressor.h"
#include "DecompressorProxyReader.h"

CompactProLZHRLEDecompressor::CompactProLZHRLEDecompressor(int blockSize)
	: m_proxyReader(nullptr)
	, m_proxyInput(nullptr)
	, m_lzhDecompressor(nullptr)
	, m_rleDecompressor(nullptr)
	, m_blockSize(blockSize)
{
}

CompactProLZHRLEDecompressor::~CompactProLZHRLEDecompressor()
{
	delete m_rleDecompressor;
	delete m_lzhDecompressor;
	delete m_proxyReader;
	CSInputBufferFree(m_proxyInput);
}

bool CompactProLZHRLEDecompressor::Reset(CSInputBuffer *input, size_t compressedSize, size_t decompressedSize)
{
	delete m_rleDecompressor;
	delete m_lzhDecompressor;
	delete m_proxyReader;
	CSInputBufferFree(m_proxyInput);
	m_rleDecompressor = nullptr;
	m_lzhDecompressor = nullptr;
	m_proxyReader = nullptr;
	m_proxyInput = nullptr;

	m_lzhDecompressor = nullptr;
	m_proxyReader = nullptr;
	m_lzhDecompressor = new CompactProLZHDecompressor(m_blockSize);
	m_proxyReader = new DecompressorProxyReader(m_lzhDecompressor);
	// FIXME maybe: This is a really stupid workaround to compensate for the decompressor not actually supporting EOFs, and the fact that we don't know the intermediate size
	m_proxyInput = CSInputBufferAlloc(m_proxyReader, 1);
	m_rleDecompressor = new CompactProRLEDecompressor();

	if (!m_lzhDecompressor->Reset(input, compressedSize, 0))
		return false;
	if (!m_rleDecompressor->Reset(m_proxyInput, 0, decompressedSize))
		return false;

	return true;
}

bool CompactProLZHRLEDecompressor::ReadBytes(void *dest, size_t numBytes)
{
	return m_rleDecompressor->ReadBytes(dest, numBytes);
}
