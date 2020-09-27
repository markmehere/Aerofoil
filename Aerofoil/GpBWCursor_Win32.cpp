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

#include "GpBWCursor_Win32.h"
#include "GpWindows.h"

#include <stdint.h>
#include <stdlib.h>
#include <new>
#include <algorithm>

extern GpWindowsGlobals g_gpWindowsGlobals;

void GpBWCursor_Win32::Destroy()
{
	this->DecRef();
}

IGpCursor_Win32 *GpBWCursor_Win32::Create(size_t width, size_t height, const void *pixelData, const void *maskData, size_t hotSpotX, size_t hotSpotY)
{
	size_t numBits = width * height;
	size_t numBytes = (width * height + 7) / 8;
	uint8_t *convertedAndData = static_cast<uint8_t*>(malloc(numBytes));
	uint8_t *convertedXorData = static_cast<uint8_t*>(malloc(numBytes));

	if (!convertedAndData || !convertedXorData)
	{
		if (convertedAndData)
			free(convertedAndData);
		if (convertedXorData)
			free(convertedXorData);

		return nullptr;
	}

	for (size_t i = 0; i < numBytes; i++)
	{
		//           MacPx0 MacPx1
		// MacMask0  1a 0x  1a 1x
		// MacMask1  0a 1x  0a 0x
		convertedAndData[i] = static_cast<const uint8_t*>(maskData)[i] ^ 0xff;
		convertedXorData[i] = static_cast<const uint8_t*>(maskData)[i] ^ static_cast<const uint8_t*>(pixelData)[i];
	}

	HCURSOR hcursor = CreateCursor(g_gpWindowsGlobals.m_hInstance, static_cast<int>(hotSpotX), static_cast<int>(hotSpotY), static_cast<int>(width), static_cast<int>(height), convertedAndData, convertedXorData);

	free(convertedAndData);
	free(convertedXorData);

	if (!hcursor)
		return nullptr;

	void *storage = malloc(sizeof(GpBWCursor_Win32));
	if (!storage)
	{
		DestroyCursor(hcursor);
		return nullptr;
	}

	return new (storage) GpBWCursor_Win32(hcursor);
}

GpBWCursor_Win32::GpBWCursor_Win32(HCURSOR cursor)
	: m_cursor(cursor)
	, m_refCount(1)
{
}

GpBWCursor_Win32::~GpBWCursor_Win32()
{
	DestroyCursor(m_cursor);
}

const HCURSOR &GpBWCursor_Win32::GetHCursor() const
{
	return m_cursor;
}

void GpBWCursor_Win32::IncRef()
{
	m_refCount++;
}

void GpBWCursor_Win32::DecRef()
{
	m_refCount--;
	if (m_refCount == 0)
	{
		this->~GpBWCursor_Win32();
		free(this);
	}
}
