#pragma once
#ifndef __PL_HOST_DISPLAY_DRIVER_H__
#define __PL_HOST_DISPLAY_DRIVER_H__

namespace PortabilityLayer
{
	class HostDisplayDriver
	{
	public:
		virtual void GetDisplayResolution(unsigned int &width, unsigned int &height) = 0;
		virtual void HideCursor() = 0;

		static void SetInstance(HostDisplayDriver *instance);
		static HostDisplayDriver *GetInstance();

	private:
		static HostDisplayDriver *ms_instance;
	};
}

#endif
