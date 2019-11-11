#pragma once
#ifndef __PL_HOST_MEMORYBUFFER_H__
#define __PL_HOST_MEMORYBUFFER_H__

#include <stdint.h>

#include "CoreDefs.h"

namespace PortabilityLayer
{
	class HostMemoryBuffer
	{
	public:
		virtual void *Contents() = 0;
		virtual size_t Size() = 0;
		virtual void Destroy() = 0;
	};
}

#endif
