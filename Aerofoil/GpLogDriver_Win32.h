#pragma once

#include "IGpLogDriver.h"

namespace PortabilityLayer
{
	class IOStream;
}

class GpLogDriver_Win32 : public IGpLogDriver
{
public:
	GpLogDriver_Win32();

	static void Init();

	void VPrintf(Category category, const char *fmt, va_list args) override;
	void Shutdown() override;

	static GpLogDriver_Win32 *GetInstance();

private:
	void InitInternal();

	PortabilityLayer::IOStream *m_stream;
	bool m_isInitialized;

	static GpLogDriver_Win32 ms_instance;
};
