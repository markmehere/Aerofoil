#pragma once

class GpIOStream;
struct IGpFontHandler;

namespace PortabilityLayer
{
	class HostFontHandler
	{
	public:
		static void SetInstance(IGpFontHandler *instance);
		static IGpFontHandler *GetInstance();

	private:
		static IGpFontHandler *ms_instance;
	};
}
