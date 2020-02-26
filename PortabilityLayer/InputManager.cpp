#include "InputManager.h"
#include "MacRomanConversion.h"
#include "PLKeyEncoding.h"
#include "Vec2i.h"

#include <string.h>
#include <assert.h>

namespace PortabilityLayer
{
	class InputManagerImpl final : public InputManager
	{
	public:
		InputManagerImpl();

		const KeyDownStates *GetKeys() const override;
		void ApplyKeyboardEvent(const GpKeyboardInputEvent &vosEvent) override;
		void ApplyGamepadEvent(const GpGamepadInputEvent &vosEvent) override;
		void ApplyMouseEvent(const GpMouseInputEvent &vosEvent) override;
		int16_t GetGamepadAxis(unsigned int playerNum, GpGamepadAxis_t gamepadAxis) override;
		Vec2i GetMousePosition() const override;
		void ClearState() override;

		static InputManagerImpl *GetInstance();

	private:
		void ApplyEventAsKey(const GpKeyboardInputEvent &vosEvent, bool bit);
		void ApplyEventAsMouseButton(const GpMouseInputEvent &vosEvent, bool bit);
		void ApplyAnalogAxisEvent(const GpGamepadAnalogAxisEvent &axisEvent);

		KeyDownStates m_keyMap;
		int16_t m_axisStates[PL_INPUT_MAX_PLAYERS][GpGamepadAxes::kCount];
		Vec2i m_mousePos;

		static InputManagerImpl ms_instance;
	};

	const KeyDownStates *InputManagerImpl::GetKeys() const
	{
		return &m_keyMap;
	}

	void InputManagerImpl::ApplyKeyboardEvent(const GpKeyboardInputEvent &vosEvent)
	{
		if (vosEvent.m_eventType == GpKeyboardInputEventTypes::kDown)
			ApplyEventAsKey(vosEvent, true);
		else if (vosEvent.m_eventType == GpKeyboardInputEventTypes::kUp)
			ApplyEventAsKey(vosEvent, false);
	}

	void InputManagerImpl::ApplyGamepadEvent(const GpGamepadInputEvent &vosEvent)
	{
		if (vosEvent.m_eventType == GpGamepadInputEventTypes::kAnalogAxisChanged)
			ApplyAnalogAxisEvent(vosEvent.m_event.m_analogAxisEvent);
	}

	void InputManagerImpl::ApplyMouseEvent(const GpMouseInputEvent &vosEvent)
	{
		m_mousePos.m_x = vosEvent.m_x;
		m_mousePos.m_y = vosEvent.m_y;

		if (vosEvent.m_eventType == GpMouseEventTypes::kUp || vosEvent.m_eventType == GpMouseEventTypes::kLeave)
			this->ApplyEventAsMouseButton(vosEvent, false);
		else if (vosEvent.m_eventType == GpMouseEventTypes::kDown)
			this->ApplyEventAsMouseButton(vosEvent, true);
	}

	int16_t InputManagerImpl::GetGamepadAxis(unsigned int playerNum, GpGamepadAxis_t gamepadAxis)
	{
		assert(playerNum < PL_INPUT_MAX_PLAYERS);

		return m_axisStates[playerNum][gamepadAxis];
	}

	Vec2i InputManagerImpl::GetMousePosition() const
	{
		return m_mousePos;
	}

	void InputManagerImpl::ClearState()
	{
		memset(&m_axisStates, 0, sizeof(m_axisStates));
		memset(&m_keyMap, 0, sizeof(m_keyMap));
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
				if (MacRoman::ToUnicode(i) == vosEvent.m_key.m_unicodeChar)
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
		case GpKeyIDSubsets::kGamepadButton:
			if (vosEvent.m_key.m_gamepadKey.m_player < PL_INPUT_MAX_PLAYERS)
				m_keyMap.m_gamepadButtons[vosEvent.m_key.m_gamepadKey.m_player].Set(vosEvent.m_key.m_gamepadKey.m_button, bit);
			break;
		default:
			assert(false);
			break;
		}
	}

	void InputManagerImpl::ApplyEventAsMouseButton(const GpMouseInputEvent &vosEvent, bool bit)
	{
		m_keyMap.m_mouse.Set(vosEvent.m_button, bit);
	}

	void InputManagerImpl::ApplyAnalogAxisEvent(const GpGamepadAnalogAxisEvent &axisEvent)
	{
		if (axisEvent.m_player < PL_INPUT_MAX_PLAYERS)
			m_axisStates[axisEvent.m_player][axisEvent.m_axis] = axisEvent.m_state;
	}

	InputManagerImpl *InputManagerImpl::GetInstance()
	{
		return &ms_instance;
	}

	InputManagerImpl::InputManagerImpl()
		: m_mousePos(0, 0)
	{
		memset(m_axisStates, 0, sizeof(m_axisStates));
	}

	InputManagerImpl InputManagerImpl::ms_instance;

	InputManager *InputManager::GetInstance()
	{
		return InputManagerImpl::GetInstance();
	}
}
