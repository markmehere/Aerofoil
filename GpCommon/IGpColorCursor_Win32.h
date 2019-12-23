#pragma once

#include "IGpColorCursor.h"
#include "GpWindows.h"

struct IGpColorCursor_Win32 : public IGpColorCursor
{
public:
	virtual const HCURSOR &GetHCursor() const = 0;

	virtual void IncRef() = 0;
	virtual void DecRef() = 0;
};
