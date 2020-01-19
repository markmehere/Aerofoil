#pragma once

#include "BMPFormat.h"
#include "SharedTypes.h"

struct BitmapImage
{
	PortabilityLayer::BitmapFileHeader m_fileHeader;

	Rect GetRect() const;
};
