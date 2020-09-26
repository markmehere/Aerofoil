#pragma once

#include "IGpCursor_Win32.h"
#include "GpWindows.h"


class GpCursor_Win32 final : public IGpCursor_Win32
{
public:
	void Destroy() override;

	const HCURSOR &GetHCursor() const override;

	void IncRef() override;
	void DecRef() override;

	static IGpCursor_Win32 *Load(const wchar_t *path);

private:
	GpCursor_Win32(HCURSOR cursor);
	~GpCursor_Win32();

	HCURSOR m_cursor;
	int m_refCount;
};
