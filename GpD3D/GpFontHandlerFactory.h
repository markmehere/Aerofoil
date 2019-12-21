#pragma once

namespace PortabilityLayer
{
	class HostFontHandler;
}

class GpFontHandlerFactory final
{
public:
	static PortabilityLayer::HostFontHandler *Create();
};
