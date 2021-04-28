#pragma once

#include "IGpCursor_Win32.h"
#include "GpWindows.h"

struct IGpAllocator;

class GpBWCursor_Win32 final : public IGpCursor_Win32
{
public:
	void Destroy() override;

	const HCURSOR &GetHCursor() const override;

	void IncRef() override;
	void DecRef() override;

	static IGpCursor_Win32 *Create(IGpAllocator *alloc, size_t width, size_t height, const void *pixelData, const void *maskData, size_t hotSpotX, size_t hotSpotY);

private:
	GpBWCursor_Win32(HCURSOR cursor, IGpAllocator *alloc);
	~GpBWCursor_Win32();

	HCURSOR m_cursor;
	int m_refCount;
	IGpAllocator *m_alloc;
};
