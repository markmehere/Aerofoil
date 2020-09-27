/*
  Portions of this file based on Simple DirectMedia Layer
  Copyright (C) 1997-2020 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
	 claim that you wrote the original software. If you use this software
	 in a product, an acknowledgment in the product documentation would be
	 appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
	 misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include "GpColorCursor_Win32.h"

#include <stdint.h>
#include <stdlib.h>
#include <new>
#include <algorithm>

void GpColorCursor_Win32::Destroy()
{
	this->DecRef();
}

IGpCursor_Win32 *GpColorCursor_Win32::Create(size_t width, size_t height, const void *pixelDataRGBA, size_t hotSpotX, size_t hotSpotY)
{
	const size_t paddingBits = (sizeof(void*) * 8);

	BITMAPV4HEADER bmp;

	memset(&bmp, 0, sizeof(bmp));
	bmp.bV4Size = sizeof(bmp);
	bmp.bV4Width = width;
	bmp.bV4Height = -static_cast<LONG>(height);
	bmp.bV4Planes = 1;
	bmp.bV4BitCount = 32;
	bmp.bV4V4Compression = BI_BITFIELDS;
	bmp.bV4AlphaMask = 0xFF000000;
	bmp.bV4RedMask = 0x00FF0000;
	bmp.bV4GreenMask = 0x0000FF00;
	bmp.bV4BlueMask = 0x000000FF;

	size_t maskPitch = width + paddingBits - 1;
	maskPitch -= maskPitch % paddingBits;

	LPVOID maskBits = malloc(maskPitch * height);
	if (!maskBits)
		return nullptr;

	memset(maskBits, 0xff, maskPitch * height);

	HDC hdc = GetDC(NULL);

	LPVOID pixels;

	ICONINFO ii;
	memset(&ii, 0, sizeof(ii));
	ii.fIcon = FALSE;
	ii.xHotspot = (DWORD)hotSpotX;
	ii.yHotspot = (DWORD)hotSpotY;
	ii.hbmColor = CreateDIBSection(hdc, (BITMAPINFO*)&bmp, DIB_RGB_COLORS, &pixels, NULL, 0);
	ii.hbmMask = CreateBitmap(width, height, 1, 1, maskBits);
	ReleaseDC(NULL, hdc);

	size_t cursorPitch = width * 4;

	size_t numPixels = width * height;
	memcpy(pixels, pixelDataRGBA, numPixels * 4);

	for (size_t i = 0; i < numPixels; i++)
	{
		uint8_t *pixel = static_cast<uint8_t*>(pixels) + i * 4;
		std::swap(pixel[0], pixel[2]);
	}

	HICON hicon = CreateIconIndirect(&ii);

	DeleteObject(ii.hbmColor);
	DeleteObject(ii.hbmMask);

	if (!hicon)
		return nullptr;

	void *storage = malloc(sizeof(GpColorCursor_Win32));
	if (!storage)
	{
		DestroyIcon(hicon);
		return nullptr;
	}

	return new (storage) GpColorCursor_Win32(reinterpret_cast<HCURSOR>(hicon));
}

GpColorCursor_Win32::GpColorCursor_Win32(HCURSOR cursor)
	: m_cursor(cursor)
	, m_refCount(1)
{
}

GpColorCursor_Win32::~GpColorCursor_Win32()
{
	DestroyIcon(m_cursor);
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
