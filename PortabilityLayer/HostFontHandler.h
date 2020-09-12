#pragma once

class GpIOStream;

namespace PortabilityLayer
{
	class HostFont;

	class HostFontHandler
	{
	public:
		virtual void Shutdown() = 0;

		virtual HostFont *LoadFont(GpIOStream *stream) = 0;
		virtual bool KeepStreamOpen() const = 0;

		static void SetInstance(HostFontHandler *instance);
		static HostFontHandler *GetInstance();

	private:
		static HostFontHandler *ms_instance;
	};
}
