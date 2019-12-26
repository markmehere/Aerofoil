#include "InputManager.h"
#include "MacRoman.h"
#include "PLKeyEncoding.h"

#include <string.h>
#include <assert.h>

namespace PortabilityLayer
{
	class InputManagerImpl final : public InputManager
	{
	public:
		InputManagerImpl();

		void GetKeys(KeyMap &keyMap) const override;
		void ApplyEvent(const GpKeyboardInputEvent &vosEvent) override;

		static InputManagerImpl *GetInstance();

	private:
		void ApplyEventAsKey(const GpKeyboardInputEvent &vosEvent, bool bit);
		KeyMap m_keyMap;

		static InputManagerImpl ms_instance;
	};

	void InputManagerImpl::GetKeys(KeyMap &keyMap) const
	{
		keyMap = m_keyMap;
	}

	void InputManagerImpl::ApplyEvent(const GpKeyboardInputEvent &vosEvent)
	{
		if (vosEvent.m_eventType == GpKeyboardInputEventTypes::kDown)
			ApplyEventAsKey(vosEvent, true);
		else if (vosEvent.m_eventType == GpKeyboardInputEventTypes::kUp)
			ApplyEventAsKey(vosEvent, false);
	}

	void InputManagerImpl::ApplyEventAsKey(const GpKeyboardInputEvent &vosEvent, bool bit)
	{
		switch (vosEvent.m_keyIDSubset)
		{
		case GpKeyIDSubsets::kASCII:
			assert(vosEvent.m_key.m_asciiChar >= 0 && vosEvent.m_key.m_asciiChar < 128);
			m_keyMap.m_ascii.Set(vosEvent.m_key.m_asciiChar, bit);
			break;
		case GpKeyIDSubsets::kUnicode:
			for (int i = 0; i < 256; i++)
			{
				if (MacRoman::g_toUnicode[i] == vosEvent.m_key.m_unicodeChar)
				{
					if (i < 128)
						m_keyMap.m_ascii.Set(i, bit);
					else
						m_keyMap.m_macRoman.Set(i - 128, bit);
					break;
				}
			}
			break;
		case GpKeyIDSubsets::kSpecial:
			m_keyMap.m_special.Set(vosEvent.m_key.m_specialKey, bit);
			break;
		case GpKeyIDSubsets::kNumPadNumber:
			m_keyMap.m_numPadNumber.Set(vosEvent.m_key.m_numPadNumber, bit);
			break;
		case GpKeyIDSubsets::kNumPadSpecial:
			m_keyMap.m_numPadSpecial.Set(vosEvent.m_key.m_numPadSpecialKey, bit);
			break;
		case GpKeyIDSubsets::kFKey:
			m_keyMap.m_fKey.Set(vosEvent.m_key.m_fKey - 1, bit);
			break;
		default:
			assert(false);
			break;
		}
	}

	InputManagerImpl *InputManagerImpl::GetInstance()
	{
		return &ms_instance;
	}

	InputManagerImpl::InputManagerImpl()
	{
	}

	InputManagerImpl InputManagerImpl::ms_instance;

	InputManager *InputManager::GetInstance()
	{
		return InputManagerImpl::GetInstance();
	}
}
