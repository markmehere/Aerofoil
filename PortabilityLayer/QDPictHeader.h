#pragma once

#include <stdint.h>
#include "SharedTypes.h"

namespace PortabilityLayer
{
	class IOStream;

	class QDPictHeader
	{
	public:
		QDPictHeader();
		bool Load(IOStream *stream);

		int GetVersion() const;
		const Rect &GetFrame() const;

	private:
		Rect m_frame;

		int m_pictVersion;
	};
}
