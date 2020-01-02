#pragma once

#include <stdint.h>

template<class T>
class THandle;

namespace PortabilityLayer
{
	class PixMapImpl;
	class SimpleImage;

	class IconLoader
	{
	public:
		virtual bool LoadColorIcon(const int16_t id, THandle<PixMapImpl> &outColorImage, THandle<PixMapImpl> &outBWImage, THandle<PixMapImpl> &outMaskImage) = 0;
		virtual THandle<PixMapImpl> LoadBWIcon(const int16_t id) = 0;

		static IconLoader *GetInstance();
	};
}
