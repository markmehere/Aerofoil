#include "WindowDef.h"
#include "IOStream.h"
#include "CoreDefs.h"

namespace PortabilityLayer
{
	bool WindowDef::Deserialize(IOStream *stream)
	{
		struct WindowDefPart1
		{
			BERect m_initialRect;
			BEInt16_t m_wdefResID;
			BEUInt16_t m_visibilityStatus;
			BEUInt16_t m_hasCloseBox;
			BEUInt32_t m_referenceConstant;
			uint8_t m_titleLength;
		};

		PL_STATIC_ASSERT(sizeof(WindowDefPart1) == 19);

		WindowDefPart1 wdefPart1;

		if (stream->Read(&wdefPart1, sizeof(wdefPart1)) != sizeof(wdefPart1))
			return false;

		m_initialRect = wdefPart1.m_initialRect.ToRect();
		m_wdefResID = wdefPart1.m_wdefResID;
		m_visibilityStatus = wdefPart1.m_visibilityStatus;
		m_hasCloseBox = wdefPart1.m_hasCloseBox;
		m_referenceConstant = wdefPart1.m_referenceConstant;

		m_title[0] = wdefPart1.m_titleLength;
		if (stream->Read(m_title + 1, wdefPart1.m_titleLength) != wdefPart1.m_titleLength)
			return false;

		if (stream->Read(&m_positionSpec, 2) == 2)
			ByteSwap::BigUInt16(m_positionSpec);
		else
			m_positionSpec = 0;

		return true;
	}
}
