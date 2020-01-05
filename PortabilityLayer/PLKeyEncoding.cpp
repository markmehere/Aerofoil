#include "PLKeyEncoding.h"

#include <assert.h>

static bool BitTestEitherSpecial(const KeyDownStates &keyMap, int eitherSpecial)
{
	switch (eitherSpecial)
	{
	case KeyEventEitherSpecialCategories::kAlt:
		return keyMap.m_special.Get(GpKeySpecials::kLeftAlt) || keyMap.m_special.Get(GpKeySpecials::kRightAlt);
	case KeyEventEitherSpecialCategories::kShift:
		return keyMap.m_special.Get(GpKeySpecials::kLeftShift) || keyMap.m_special.Get(GpKeySpecials::kRightShift);
	case KeyEventEitherSpecialCategories::kControl:
		return keyMap.m_special.Get(GpKeySpecials::kLeftCtrl) || keyMap.m_special.Get(GpKeySpecials::kRightCtrl);
	default:
		assert(false);
		return false;
	}
}

bool KeyDownStates::IsSet(intptr_t encodedKey) const
{
	const KeyEventType evtType = PL_KEY_GET_EVENT_TYPE(encodedKey);
	const int evtValue = PL_KEY_GET_VALUE(encodedKey);

	switch (evtType)
	{
	case KeyEventType_Special:
		return m_special.Get(evtValue);
	case KeyEventType_ASCII:
		return m_ascii.Get(evtValue);
	case KeyEventType_MacRoman:
		assert(evtValue >= 128 && evtValue < 256);
		return m_macRoman.Get(evtValue - 128);
	case KeyEventType_NumPadNumber:
		return m_numPadNumber.Get(evtValue);
	case KeyEventType_NumPadSpecial:
		return m_numPadSpecial.Get(evtValue);
	case KeyEventType_FKey:
		assert(evtValue >= 1 && evtValue <= GpFKeyMaximumInclusive);
		return m_fKey.Get(evtValue - 1);
	case KeyEventType_EitherSpecial:
		return BitTestEitherSpecial(*this, evtValue);
	case KeyEventType_GamepadButton:
	{
		unsigned int playerNum = evtValue & ((1 << PL_INPUT_PLAYER_INDEX_BITS) - 1);
		assert(playerNum < PL_INPUT_MAX_PLAYERS);
		unsigned int button = evtValue >> PL_INPUT_PLAYER_INDEX_BITS;

		return m_gamepadButtons[playerNum].Get(button);
	}
	break;
	default:
		assert(false);
		return false;
	}
}
