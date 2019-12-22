#pragma once

#include "IGpColorCursor.h"
#include "GpWindows.h"

class GpColorCursor_Win32 final : public IGpColorCursor
{
public:
	void Destroy() override;

	const HCURSOR &GetHCursor() const;

	void IncRef();
	void DecRef();

	static GpColorCursor_Win32 *Load(const wchar_t *path);

private:
	GpColorCursor_Win32(HCURSOR cursor);
	~GpColorCursor_Win32();

	HCURSOR m_cursor;
	int m_refCount;
};
