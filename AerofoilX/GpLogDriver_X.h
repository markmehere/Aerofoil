#pragma once

#include "IGpLogDriver.h"

#include <stdio.h>

class GpIOStream;

class GpLogDriver_X : public IGpLogDriver
{
public:
	GpLogDriver_X();

	static void Init();

	void VPrintf(Category category, const char *fmt, va_list args) override;
	void Shutdown() override;

	static GpLogDriver_X *GetInstance();

private:
	void InitInternal();

	GpIOStream *m_stream;
	bool m_isInitialized;

	static GpLogDriver_X ms_instance;
};
