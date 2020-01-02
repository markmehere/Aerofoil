#include "WindowDef.h"
#include "IOStream.h"
#include "CoreDefs.h"
#include "PLPasStr.h"

#include <algorithm>

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

		GP_STATIC_ASSERT(sizeof(WindowDefPart1) == 19);

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

	WindowDef WindowDef::Create(const Rect &initialRect, int16_t wdefID, bool isVisible, bool hasCloseBox, uint32_t refConstant, uint16_t positionSpec, const PLPasStr &title)
	{
		WindowDef wdef;
		wdef.m_initialRect = initialRect;
		wdef.m_wdefResID = wdefID;
		wdef.m_visibilityStatus = isVisible ? 1 : 0;
		wdef.m_hasCloseBox = hasCloseBox ? 1 : 0;
		wdef.m_referenceConstant = refConstant;
		wdef.m_positionSpec = positionSpec;

		const uint8_t titleLength = static_cast<uint8_t>(std::max<size_t>(255, title.Length()));
		wdef.m_title[0] = titleLength;
		memcpy(wdef.m_title + 1, title.UChars(), titleLength);

		return wdef;
	}
}
