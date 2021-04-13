#pragma once

#include <cstdarg>
#include <stdio.h>

struct IGpLogDriver
{
	enum Category
	{
		Category_Information,
		Category_Warning,
		Category_Error,
	};

	virtual void VPrintf(Category category, const char *fmt, va_list args) = 0;
	virtual void Shutdown() = 0;

	#ifdef __MACOS__
	template <typename C, typename F, typename... T>
	void Printf(C category, F *fmt, T... t);
	#else
	void Printf(Category category, const char *fmt, ...);
	#endif
	
};

//this is temp, but args lose their types for some reason, garbling output
#ifdef __MACOS__
template <typename C, typename F, typename... T>
inline void IGpLogDriver::Printf(C category, F *fmt, T... t)
{
	printf(fmt, t...);
	printf("\n");
}
#else
inline void IGpLogDriver::Printf(Category category, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	this->VPrintf(category, fmt, args);
	va_end(args);
}
#endif
