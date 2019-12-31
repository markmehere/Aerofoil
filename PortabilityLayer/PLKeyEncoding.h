#pragma once

#include "GpVOSEvent.h"
#include "GpBitfield.h"

struct GpKeyboardInputEvent;

enum KeyEventType
{
	KeyEventType_Special,
	KeyEventType_ASCII,
	KeyEventType_MacRoman,
	KeyEventType_NumPadNumber,
	KeyEventType_NumPadSpecial,
	KeyEventType_FKey,
	KeyEventType_EitherSpecial,
	KeyEventType_GamepadButton,

	KeyEventType_Count,
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

#define PL_INPUT_MAX_PLAYERS			2
#define PL_INPUT_PLAYER_INDEX_BITS		1

#define PL_INPUT_TYPE_CODE_BITS			3

#define PL_KEY_SPECIAL(k)					((KeyEventType_Special) | ((GpKeySpecials::k) << PL_INPUT_TYPE_CODE_BITS))
#define PL_KEY_SPECIAL_ENCODE(k)			((KeyEventType_Special) | ((k) << PL_INPUT_TYPE_CODE_BITS))
#define PL_KEY_ASCII(k)						((KeyEventType_ASCII) | ((k) << PL_INPUT_TYPE_CODE_BITS))
#define PL_KEY_MACROMAN(k)					((KeyEventType_MacRoman) | ((k) << PL_INPUT_TYPE_CODE_BITS))
#define PL_KEY_NUMPAD_NUMBER(k)				((KeyEventType_NumPadNumber) | ((k) << PL_INPUT_TYPE_CODE_BITS))
#define PL_KEY_NUMPAD_SPECIAL(k)			((KeyEventType_NumPadSpecial) | ((GpKeySpecials::k) << PL_INPUT_TYPE_CODE_BITS))
#define PL_KEY_NUMPAD_SPECIAL_ENCODE(k)		((KeyEventType_NumPadSpecial) | ((k) << PL_INPUT_TYPE_CODE_BITS))
#define PL_KEY_FKEY(k)						((KeyEventType_FKey) | ((k) << PL_INPUT_TYPE_CODE_BITS))
#define PL_KEY_EITHER_SPECIAL(k)			((KeyEventType_EitherSpecial) | ((KeyEventEitherSpecialCategories::k) << PL_INPUT_TYPE_CODE_BITS))
#define PL_KEY_GAMEPAD_BUTTON(k, pl)		((KeyEventType_GamepadButton) | (pl << PL_INPUT_TYPE_CODE_BITS) | ((GpGamepadButtons::k) << (PL_INPUT_TYPE_CODE_BITS + PL_INPUT_PLAYER_INDEX_BITS)))
#define PL_KEY_GAMEPAD_BUTTON_ENCODE(k, pl)	((KeyEventType_GamepadButton) | (pl << PL_INPUT_TYPE_CODE_BITS) | ((k) << (PL_INPUT_TYPE_CODE_BITS + PL_INPUT_PLAYER_INDEX_BITS)))

#define PL_KEY_GET_EVENT_TYPE(k)			(static_cast<KeyEventType>(k & ((1 << PL_INPUT_TYPE_CODE_BITS) - 1)))
#define PL_KEY_GET_VALUE(k)					((k) >> PL_INPUT_TYPE_CODE_BITS)

intptr_t PackVOSKeyCode(const GpKeyboardInputEvent &evt);

struct KeyDownStates
{
	GpBitfield<GpKeySpecials::kCount> m_special;
	GpBitfield<128> m_ascii;
	GpBitfield<128> m_macRoman;
	GpBitfield<10> m_numPadNumber;
	GpBitfield<GpNumPadSpecials::kCount> m_numPadSpecial;
	GpBitfield<GpFKeyMaximumInclusive> m_fKey;
	GpBitfield<GpGamepadButtons::kCount> m_gamepadButtons[PL_INPUT_MAX_PLAYERS];
};
