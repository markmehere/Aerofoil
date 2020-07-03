#pragma once

#include <stdint.h>

struct IGpPrefsHandler
{
	typedef bool (*WritePrefsFunc_t) (void *context, const void *identifier, size_t identifierSize, const void *contents, size_t contentsSize, uint32_t version);

	virtual void ApplyPrefs(const void *identifier, size_t identifierSize, const void *contents, size_t contentsSize, uint32_t version) = 0;
	virtual bool SavePrefs(void *context, WritePrefsFunc_t writeFunc) = 0;
};
