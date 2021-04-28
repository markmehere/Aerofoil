#pragma once

#include "IGpAllocator.h"

class GpAllocator_C final : public IGpAllocator
{
public:
	void *Realloc(void *buf, size_t newSize) override;

	static GpAllocator_C *GetInstance();

private:
	void *Alloc(size_t size);
	void Free(void *ptr);

	static GpAllocator_C ms_instance;
};
