#include "AppEventHandler.h"

namespace PortabilityLayer
{
	IAppEventHandler *AppEventHandler::ms_instance;

	IAppEventHandler *AppEventHandler::GetInstance()
	{
		return ms_instance;
	}

	void AppEventHandler::SetInstance(IAppEventHandler *instance)
	{
		ms_instance = instance;
	}
}
