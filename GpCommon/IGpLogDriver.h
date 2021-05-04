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

	void Printf(Category category, const char *fmt, ...);
};

inline void IGpLogDriver::Printf(Category category, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	this->VPrintf(category, fmt, args);
	va_end(args);
}
