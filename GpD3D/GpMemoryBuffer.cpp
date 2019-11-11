#include "GpMemoryBuffer.h"

#include <new>

void *GpMemoryBuffer::Contents()
{
	return reinterpret_cast<uint8_t*>(this) + AlignedSize();
}

size_t GpMemoryBuffer::Size()
{
	return m_size;
}

void GpMemoryBuffer::Destroy()
{
	delete[] reinterpret_cast<uint8_t*>(this);
}

GpMemoryBuffer *GpMemoryBuffer::Create(size_t sz)
{
	const size_t allowedSize = SIZE_MAX - AlignedSize();
	if (sz > allowedSize)
		return nullptr;

	const size_t bufferSize = GpMemoryBuffer::AlignedSize() + sz;

	uint8_t *buffer = new uint8_t[bufferSize];
	new (buffer) GpMemoryBuffer(sz);

	return reinterpret_cast<GpMemoryBuffer*>(buffer);
}

GpMemoryBuffer::GpMemoryBuffer(size_t sz)
	: m_size(sz)
{
}

GpMemoryBuffer::~GpMemoryBuffer()
{
}

size_t GpMemoryBuffer::AlignedSize()
{
	const size_t paddedSize = (sizeof(GpMemoryBuffer) + PL_SYSTEM_MEMORY_ALIGNMENT - 1);
	const size_t sz = paddedSize - paddedSize % PL_SYSTEM_MEMORY_ALIGNMENT;

	return sz;
}
