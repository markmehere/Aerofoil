#pragma once

#include "IGpMutex.h"

#include "GpWindows.h"

struct IGpAllocator;

class GpMutex_Win32 final : public IGpMutex
{
public:
	void Destroy() override;
	void Lock() override;
	void Unlock() override;

	static GpMutex_Win32 *Create(IGpAllocator *alloc);

private:
	explicit GpMutex_Win32(IGpAllocator *alloc);
	~GpMutex_Win32();

	CRITICAL_SECTION m_critSection;
	IGpAllocator *m_alloc;
};
