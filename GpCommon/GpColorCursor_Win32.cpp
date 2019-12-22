#include "GpColorCursor_Win32.h"

#include <stdlib.h>
#include <new>

void GpColorCursor_Win32::Destroy()
{
	this->DecRef();
}

GpColorCursor_Win32 *GpColorCursor_Win32::Load(const wchar_t *path)
{
	HANDLE imageH = LoadImageW(nullptr, path, IMAGE_CURSOR, 0, 0, LR_LOADFROMFILE);

	if (imageH == nullptr)
		return nullptr;

	HCURSOR cursor = reinterpret_cast<HCURSOR>(imageH);
	void *storage = malloc(sizeof(GpColorCursor_Win32));
	if (!storage)
	{
		DestroyCursor(cursor);
		return nullptr;
	}

	return new (storage) GpColorCursor_Win32(reinterpret_cast<HCURSOR>(cursor));
}

GpColorCursor_Win32::GpColorCursor_Win32(HCURSOR cursor)
	: m_cursor(cursor)
	, m_refCount(1)
{
}

GpColorCursor_Win32::~GpColorCursor_Win32()
{
	DestroyCursor(m_cursor);
}

const HCURSOR &GpColorCursor_Win32::GetHCursor() const
{
	return m_cursor;
}

void GpColorCursor_Win32::IncRef()
{
	m_refCount++;
}

void GpColorCursor_Win32::DecRef()
{
	m_refCount--;
	if (m_refCount == 0)
	{
		this->~GpColorCursor_Win32();
		free(this);
	}
}
