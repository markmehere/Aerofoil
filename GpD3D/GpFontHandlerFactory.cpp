#include "GpFontHandlerFactory.h"
#include "GpFontHandler_FreeType2.h"

#include <stdlib.h>

PortabilityLayer::HostFontHandler *GpFontHandlerFactory::Create()
{
	return GpFontHandler_FreeType2::Create();
}
