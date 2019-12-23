#pragma once

#include "IGpColorCursor_Win32.h"
#include "GpWindows.h"


class GpColorCursor_Win32 final : public IGpColorCursor_Win32
{
public:
	void Destroy() override;

	const HCURSOR &GetHCursor() const override;

	void IncRef() override;
	void DecRef() override;

	static GpColorCursor_Win32 *Load(const wchar_t *path);

private:
	GpColorCursor_Win32(HCURSOR cursor);
	~GpColorCursor_Win32();

	HCURSOR m_cursor;
	int m_refCount;
};
