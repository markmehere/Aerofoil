#pragma once

#include "GpIOStream.h"

namespace PortabilityLayer
{
	namespace FileSectionStream
	{
		GpIOStream *Create(GpIOStream *stream, GpUFilePos_t start, GpUFilePos_t size);
	};
}
