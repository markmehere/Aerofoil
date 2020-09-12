#pragma once

#include "EGpFontHandlerType.h"

namespace PortabilityLayer
{
	class HostFontHandler;
}

struct GpFontHandlerProperties;

class GpFontHandlerFactory
{
public:
	typedef PortabilityLayer::HostFontHandler *(*FactoryFunc_t)(const GpFontHandlerProperties &properties);

	static PortabilityLayer::HostFontHandler *CreateFontHandler(const GpFontHandlerProperties &properties);
	static void RegisterFontHandlerFactory(EGpFontHandlerType type, FactoryFunc_t func);

private:
	static FactoryFunc_t ms_registry[EGpFontHandlerType_Count];
};
