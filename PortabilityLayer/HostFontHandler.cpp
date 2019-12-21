#include "HostFontHandler.h"

namespace PortabilityLayer
{
	void HostFontHandler::SetInstance(HostFontHandler *instance)
	{
		ms_instance = instance;
	}

	HostFontHandler *HostFontHandler::GetInstance()
	{
		return ms_instance;
	}

	HostFontHandler *HostFontHandler::ms_instance = nullptr;
}
