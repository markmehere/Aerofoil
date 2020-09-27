#pragma once

#include "IGpCursor_Win32.h"
#include "GpWindows.h"


class GpColorCursor_Win32 final : public IGpCursor_Win32
{
public:
	void Destroy() override;

	const HCURSOR &GetHCursor() const override;

	void IncRef() override;
	void DecRef() override;

	static IGpCursor_Win32 *Create(size_t width, size_t height, const void *pixelDataRGBA, size_t hotSpotX, size_t hotSpotY);

private:
	GpColorCursor_Win32(HCURSOR cursor);
	~GpColorCursor_Win32();

	HCURSOR m_cursor;
	int m_refCount;
};
