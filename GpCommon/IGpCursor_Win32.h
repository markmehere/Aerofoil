#pragma once

#include "IGpCursor.h"
#include "GpWindows.h"

struct IGpCursor_Win32 : public IGpCursor
{
public:
	virtual const HCURSOR &GetHCursor() const = 0;

	virtual void IncRef() = 0;
	virtual void DecRef() = 0;
};
