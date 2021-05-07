#pragma once

#include <stddef.h>

namespace PortabilityLayer
{
	class ResTypeID;

	struct GpArcResourceTypeTag
	{
		char m_id[13];

		static GpArcResourceTypeTag Encode(const ResTypeID &tag);

		bool Load(const char *str, size_t strLen);
		bool Decode(ResTypeID &outTag);
	};
}
