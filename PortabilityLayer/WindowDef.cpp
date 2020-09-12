#include "WindowDef.h"
#include "GpIOStream.h"
#include "CoreDefs.h"
#include "PLPasStr.h"

#include <algorithm>

namespace PortabilityLayer
{
	bool WindowDef::Deserialize(GpIOStream *stream)
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

		enum WDefPositionSpecs
		{
			kPosSpec_CommonBits					= 0x200a,

			kPosSpec_LocationMask				= 0x1800,
			kPosSpec_Location_Center			= 0x0800,
			kPosSpec_Location_AlertPosition		= 0x1000,
			kPosSpec_Location_Stagger			= 0x1800,

			kPosSpec_SubsetMask					= 0xc000,
			kPosSpec_Subset_MainScreen			= 0x0000,
			kPosSpec_Subset_ParentWindow		= 0x8000,
			kPosSpec_Subset_ParentWindowsScreen	= 0x4000,
		};

		enum Styles
		{
			kStyle_BarResizable				= 0,
			kStyle_Bar						= 4,
			kStyle_BarResizableExpandable	= 8,
			kStyle_BarExpandable			= 12,
			kStyle_Box						= 2,
			kStyle_BoxShadow				= 3,
			kStyle_BarCloseBoxBlack			= 16,
			kStyle_Alert					= 1,
			kStyle_BarNoMinimize			= 5,
		};

		GP_STATIC_ASSERT(sizeof(WindowDefPart1) == 19);

		WindowDefPart1 wdefPart1;

		if (stream->Read(&wdefPart1, sizeof(wdefPart1)) != sizeof(wdefPart1))
			return false;

		uint16_t styleFlags = 0;

		switch (static_cast<int>(wdefPart1.m_wdefResID))
		{
		case kStyle_Bar:
		case kStyle_BarNoMinimize:
			styleFlags = WindowStyleFlags::kTitleBar;
			break;
		case kStyle_Box:
			styleFlags = 0;
			break;
		default:
			return false;	// Unsupported window style
		}

		if (wdefPart1.m_hasCloseBox != 0)
			styleFlags |= WindowStyleFlags::kCloseBox;

		m_initialRect = wdefPart1.m_initialRect.ToRect();
		m_styleFlags = styleFlags;
		m_visibilityStatus = wdefPart1.m_visibilityStatus;
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

	WindowDef WindowDef::Create(const Rect &initialRect, uint16_t styleFlags, bool isVisible, uint32_t refConstant, uint16_t positionSpec, const PLPasStr &title)
	{
		WindowDef wdef;
		wdef.m_initialRect = initialRect;
		wdef.m_styleFlags = styleFlags;
		wdef.m_visibilityStatus = isVisible ? 1 : 0;
		wdef.m_referenceConstant = refConstant;
		wdef.m_positionSpec = positionSpec;

		const uint8_t titleLength = static_cast<uint8_t>(std::min<size_t>(255, title.Length()));
		wdef.m_title[0] = titleLength;
		memcpy(wdef.m_title + 1, title.UChars(), titleLength);

		return wdef;
	}
}
