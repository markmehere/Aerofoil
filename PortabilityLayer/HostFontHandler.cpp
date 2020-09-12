#include "HostFontHandler.h"

namespace PortabilityLayer
{
	void HostFontHandler::SetInstance(IGpFontHandler *instance)
	{
		ms_instance = instance;
	}

	IGpFontHandler *HostFontHandler::GetInstance()
	{
		return ms_instance;
	}

	IGpFontHandler *HostFontHandler::ms_instance = nullptr;
}
