#pragma once

#include "EGpDisplayDriverType.h"

class IGpDisplayDriver;
struct GpDisplayDriverProperties;

class GpDisplayDriverFactory
{
public:
	typedef IGpDisplayDriver *(*FactoryFunc_t)(const GpDisplayDriverProperties &properties);

	static IGpDisplayDriver *CreateDisplayDriver(const GpDisplayDriverProperties &properties);
	static void RegisterDisplayDriverFactory(EGpDisplayDriverType type, FactoryFunc_t func);

private:
	static FactoryFunc_t ms_Registry[EGpDisplayDriverType_Count];
};
