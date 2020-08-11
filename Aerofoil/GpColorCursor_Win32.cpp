#include "GpCursor_Win32.h"

#include <stdlib.h>
#include <new>

void GpCursor_Win32::Destroy()
{
	this->DecRef();
}

IGpCursor_Win32 *GpCursor_Win32::Load(const wchar_t *path)
{
	HANDLE imageH = LoadImageW(nullptr, path, IMAGE_CURSOR, 0, 0, LR_LOADFROMFILE);

	if (imageH == nullptr)
		return nullptr;

	HCURSOR cursor = reinterpret_cast<HCURSOR>(imageH);
	void *storage = malloc(sizeof(GpCursor_Win32));
	if (!storage)
	{
		DestroyCursor(cursor);
		return nullptr;
	}

	return new (storage) GpCursor_Win32(reinterpret_cast<HCURSOR>(cursor));
}

GpCursor_Win32::GpCursor_Win32(HCURSOR cursor)
	: m_cursor(cursor)
	, m_refCount(1)
{
}

GpCursor_Win32::~GpCursor_Win32()
{
	DestroyCursor(m_cursor);
}

const HCURSOR &GpCursor_Win32::GetHCursor() const
{
	return m_cursor;
}

void GpCursor_Win32::IncRef()
{
	m_refCount++;
}

void GpCursor_Win32::DecRef()
{
	m_refCount--;
	if (m_refCount == 0)
	{
		this->~GpCursor_Win32();
		free(this);
	}
}
