#pragma once

#include "HostMemoryBuffer.h"

class GpMemoryBuffer final : public PortabilityLayer::HostMemoryBuffer
{
public:
	void *Contents() override;
	size_t Size() override;
	void Destroy() override;

	static GpMemoryBuffer *Create(size_t sz);

private:
	explicit GpMemoryBuffer(size_t sz);
	~GpMemoryBuffer();

	static size_t AlignedSize();

	size_t m_size;
};
