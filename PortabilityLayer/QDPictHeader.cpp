#include "QDPictHeader.h"
#include "IOStream.h"
#include "SharedTypes.h"
#include "CoreDefs.h"

#include <string.h>
#include <assert.h>

namespace PortabilityLayer
{
	QDPictHeader::QDPictHeader()
		: m_pictVersion(0)
	{
	}

	bool QDPictHeader::Load(IOStream *stream)
	{
		GP_STATIC_ASSERT(sizeof(Picture) == 10);

		Picture pictHeader;
		if (stream->Read(&pictHeader, sizeof(Picture)) != sizeof(Picture))
			return false;

		m_frame = pictHeader.picFrame.ToRect();
		if (!m_frame.IsValid())
			return false;

		uint8_t versionTag1[2];
		if (stream->Read(versionTag1, 2) != 2)
			return false;

		if (versionTag1[0] == 0x11 && versionTag1[1] == 0x01)
		{
			m_pictVersion = 1;
		}
		else if (versionTag1[0] == 0x00 && versionTag1[1] == 0x11)
		{
			m_pictVersion = 2;

			uint8_t v2Header[28];
			if (stream->Read(v2Header, 28) != 28)
				return false;

			if (v2Header[0] != 0x02 || v2Header[1] != 0xff || v2Header[2] != 0x0c || v2Header[3] != 0x00)
				return false;

			BEInt16_t v2Version;
			memcpy(&v2Version, v2Header + 4, 2);

			// In version 2 header, v2Version == -1
			// Followed by fixed-point bounding rectangle (16 bytes) and 4 reserved
			// In ext. version 2 header, v2Version == -2
			// Followed by 2-byte reserved, horizontal DPI (fixed point, 4 bytes), vertical DPI (fixed point, 4 bytes) optimal source rect (8 bytes), and 2 reserved
		}
		else
			return false;

		return true;
	}

	int QDPictHeader::GetVersion() const
	{
		return m_pictVersion;
	}

	const Rect &QDPictHeader::GetFrame() const
	{
		return m_frame;
	}
}