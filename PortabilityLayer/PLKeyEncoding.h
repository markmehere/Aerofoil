#pragma once

#include "GpVOSEvent.h"
#include "GpBitfield.h"

enum KeyEventType
{
	KeyEventType_Special,
	KeyEventType_ASCII,
	KeyEventType_MacRoman,
	KeyEventType_NumPadNumber,
	KeyEventType_NumPadSpecial,
	KeyEventType_FKey,

	KeyEventType_EitherSpecial,
};

namespace KeyEventEitherSpecialCategories
{
	enum KeyEventEitherSpecialCategory
	{
		kControl,
		kAlt,
		kShift,
	};
}

#define PL_KEY_SPECIAL(k)				((KeyEventType_Special) | (GpKeySpecials::k) << 3)
#define PL_KEY_SPECIAL_ENCODE(k)		((KeyEventType_Special) | (k) << 3)
#define PL_KEY_ASCII(k)					((KeyEventType_ASCII) | (k) << 3)
#define PL_KEY_MACROMAN(k)				((KeyEventType_MacRoman) | (k) << 3)
#define PL_KEY_NUMPAD_NUMBER(k)			((KeyEventType_NumPadNumber) | (k) << 3)
#define PL_KEY_NUMPAD_SPECIAL(k)		((KeyEventType_NumPadSpecial) | (GpKeySpecials::k) << 3)
#define PL_KEY_NUMPAD_SPECIAL_ENCODE(k)	((KeyEventType_NumPadSpecial) | (k) << 3)
#define PL_KEY_FKEY(k)					((KeyEventType_FKey) | (k) << 3)
#define PL_KEY_EITHER_SPECIAL(k)		((KeyEventType_EitherSpecial) | (KeyEventEitherSpecialCategories::k) << 3)

#define PL_KEY_GET_EVENT_TYPE(k)	(static_cast<KeyEventType>(k & 7))
#define PL_KEY_GET_VALUE(k)			((k) >> 3)

struct KeyMap
{
	GpBitfield<GpKeySpecials::kCount> m_special;
	GpBitfield<128> m_ascii;
	GpBitfield<128> m_macRoman;
	GpBitfield<10> m_numPadNumber;
	GpBitfield<GpNumPadSpecials::kCount> m_numPadSpecial;
	GpBitfield<GpFKeyMaximumInclusive> m_fKey;
};
