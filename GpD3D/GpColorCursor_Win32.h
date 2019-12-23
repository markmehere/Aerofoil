#pragma once

#include "IGpColorCursor.h"
#include "GpWindows.h"

struct IGpColorCursor_Win32 : public IGpColorCursor
{
	virtual const HCURSOR &GetHCursor() const = 0;

	virtual void IncRef() = 0;
	virtual void DecRef() = 0;
};


class GpColorCursor_Win32 final : public IGpColorCursor_Win32
{
public:
	void Destroy() override;

	const HCURSOR &GetHCursor() const override;

	void IncRef() override;
	void DecRef() override;

	static IGpColorCursor_Win32 *Load(const wchar_t *path);

private:
	GpColorCursor_Win32(HCURSOR cursor);
	~GpColorCursor_Win32();

	HCURSOR m_cursor;
	int m_refCount;
};
