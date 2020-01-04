#include "PLApplication.h"
#include "PLCore.h"
#include "HostSystemServices.h"

#include <string.h>
#include <assert.h>

namespace PortabilityLayer
{
	namespace Utils
	{
		void MakePStr(unsigned char *dest, size_t sz, const char *src)
		{
			assert(sz <= 255);
			dest[0] = static_cast<uint8_t>(sz);
			memcpy(dest + 1, src, sz);
		}
	}
}

void SysBeep(int duration)
{
	PortabilityLayer::HostSystemServices::GetInstance()->Beep();
}
