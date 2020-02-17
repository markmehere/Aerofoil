#include "GpInputDriverXInput.h"
#include "GpVOSEvent.h"
#include "GpWindows.h"
#include "IGpVOSEventQueue.h"

#include <stdlib.h>
#include <new>

#pragma comment(lib, "xinput9_1_0.lib")

void GpInputDriverXInput::ProcessInput()
{
	for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)
	{
		XINPUT_STATE newState;
		memset(&newState, 0, sizeof(newState));

		DWORD result = XInputGetState(i, &newState);

		if (result == ERROR_SUCCESS)
		{
			bool isNewState = false;
			unsigned int playerNumber = 0;
			if (m_xUserToPlayerNumber[i] < 0)
			{
				// Newly-connected player

				for (int i = 0; i < XUSER_MAX_COUNT; i++)
				{
					if (!m_playerNumberIsConnected[i])
					{
						playerNumber = i;
						break;
					}
				}

				m_xUserToPlayerNumber[i] = playerNumber;
				m_playerNumberIsConnected[playerNumber] = true;

				isNewState = true;
			}
			else
			{
				playerNumber = m_xUserToPlayerNumber[i];
				isNewState = (newState.dwPacketNumber != m_controllerStates[i].dwPacketNumber);
			}

			if (isNewState)
			{
				this->ProcessControllerStateChange(playerNumber, m_controllerStates[i], newState);
				m_controllerStates[i] = newState;
			}
		}
		else
		{
			if (m_xUserToPlayerNumber[i] >= 0)
			{
				// Was connected last frame, process as a blank state and then free the player slot
				memset(&newState, 0, sizeof(newState));

				const unsigned int playerNumber = m_xUserToPlayerNumber[i];

				this->ProcessControllerStateChange(playerNumber, m_controllerStates[i], newState);

				m_controllerStates[i] = newState;
				m_playerNumberIsConnected[playerNumber] = false;
				m_xUserToPlayerNumber[i] = -1;
			}
		}
	}
}

void GpInputDriverXInput::Shutdown()
{
	this->~GpInputDriverXInput();
	free(this);
}

GpInputDriverXInput *GpInputDriverXInput::Create(const GpInputDriverProperties &props)
{
	void *storage = malloc(sizeof(GpInputDriverXInput));
	if (!storage)
		return nullptr;

	return new (storage) GpInputDriverXInput(props);
}

GpInputDriverXInput::GpInputDriverXInput(const GpInputDriverProperties &props)
	: m_properties(props)
{
	for (int i = 0; i < XUSER_MAX_COUNT; i++)
		m_xUserToPlayerNumber[i] = -1;

	memset(m_controllerStates, 0, sizeof(m_controllerStates));
	memset(m_playerNumberIsConnected, 0, sizeof(m_playerNumberIsConnected));
}

GpInputDriverXInput::~GpInputDriverXInput()
{
}

void GpInputDriverXInput::ProcessControllerStateChange(unsigned int playerNumber, const XINPUT_STATE &prevState, const XINPUT_STATE &newState)
{
	const XINPUT_GAMEPAD &prevGamepad = prevState.Gamepad;
	const XINPUT_GAMEPAD &newGamepad = newState.Gamepad;

	DWORD prevButtons = prevGamepad.wButtons;
	DWORD newButtons = newGamepad.wButtons;
	ProcessButtonStateChange(prevButtons, newButtons, XINPUT_GAMEPAD_START, playerNumber, GpGamepadButtons::kMisc1);
	ProcessButtonStateChange(prevButtons, newButtons, XINPUT_GAMEPAD_BACK, playerNumber, GpGamepadButtons::kMisc2);

	ProcessButtonStateChange(prevButtons, newButtons, XINPUT_GAMEPAD_DPAD_UP, playerNumber, GpGamepadButtons::kDPadUp);
	ProcessButtonStateChange(prevButtons, newButtons, XINPUT_GAMEPAD_DPAD_DOWN, playerNumber, GpGamepadButtons::kDPadDown);
	ProcessButtonStateChange(prevButtons, newButtons, XINPUT_GAMEPAD_DPAD_LEFT, playerNumber, GpGamepadButtons::kDPadLeft);
	ProcessButtonStateChange(prevButtons, newButtons, XINPUT_GAMEPAD_DPAD_RIGHT, playerNumber, GpGamepadButtons::kDPadRight);

	ProcessButtonStateChange(prevButtons, newButtons, XINPUT_GAMEPAD_LEFT_THUMB, playerNumber, GpGamepadButtons::kLeftStick);
	ProcessButtonStateChange(prevButtons, newButtons, XINPUT_GAMEPAD_RIGHT_THUMB, playerNumber, GpGamepadButtons::kRightStick);

	ProcessButtonStateChange(prevButtons, newButtons, XINPUT_GAMEPAD_LEFT_SHOULDER, playerNumber, GpGamepadButtons::kLeftBumper);
	ProcessButtonStateChange(prevButtons, newButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER, playerNumber, GpGamepadButtons::kRightBumper);

	ProcessButtonStateChange(prevButtons, newButtons, XINPUT_GAMEPAD_Y, playerNumber, GpGamepadButtons::kFaceUp);
	ProcessButtonStateChange(prevButtons, newButtons, XINPUT_GAMEPAD_A, playerNumber, GpGamepadButtons::kFaceDown);
	ProcessButtonStateChange(prevButtons, newButtons, XINPUT_GAMEPAD_X, playerNumber, GpGamepadButtons::kFaceLeft);
	ProcessButtonStateChange(prevButtons, newButtons, XINPUT_GAMEPAD_B, playerNumber, GpGamepadButtons::kFaceRight);

	ProcessAxisStateChange(ConvertTriggerValue(prevState.Gamepad.bLeftTrigger), ConvertTriggerValue(newState.Gamepad.bLeftTrigger), playerNumber, GpGamepadAxes::kLeftTrigger);
	ProcessAxisStateChange(ConvertTriggerValue(prevState.Gamepad.bRightTrigger), ConvertTriggerValue(newState.Gamepad.bRightTrigger), playerNumber, GpGamepadAxes::kRightTrigger);
	ProcessAxisStateChange(prevState.Gamepad.sThumbLX, newState.Gamepad.sThumbLX, playerNumber, GpGamepadAxes::kLeftStickX);
	ProcessAxisStateChange(prevState.Gamepad.sThumbLY, newState.Gamepad.sThumbLY, playerNumber, GpGamepadAxes::kLeftStickY);
	ProcessAxisStateChange(prevState.Gamepad.sThumbRX, newState.Gamepad.sThumbRX, playerNumber, GpGamepadAxes::kRightStickX);
	ProcessAxisStateChange(prevState.Gamepad.sThumbRY, newState.Gamepad.sThumbRY, playerNumber, GpGamepadAxes::kRightStickY);
}

void GpInputDriverXInput::ProcessButtonStateChange(DWORD prevState, DWORD newState, DWORD deltaBit, unsigned int playerNum, GpGamepadButton_t gamepadButton)
{
	DWORD deltaState = prevState ^ newState;

	if ((deltaState & deltaBit) == 0)
		return;

	const GpKeyboardInputEventType_t eventType = ((prevState & deltaBit) == 0) ? GpKeyboardInputEventTypes::kDown : GpKeyboardInputEventTypes::kUp;

	if (GpVOSEvent *evt = m_properties.m_eventQueue->QueueEvent())
	{
		evt->m_eventType = GpVOSEventTypes::kKeyboardInput;
		evt->m_event.m_keyboardInputEvent.m_eventType = eventType;
		evt->m_event.m_keyboardInputEvent.m_keyIDSubset = GpKeyIDSubsets::kGamepadButton;
		evt->m_event.m_keyboardInputEvent.m_key.m_gamepadKey.m_button = gamepadButton;
		evt->m_event.m_keyboardInputEvent.m_key.m_gamepadKey.m_player = playerNum;
	}
}

int16_t GpInputDriverXInput::ConvertTriggerValue(uint8_t v)
{
	return static_cast<int16_t>((v * 257) >> 1);
}

void GpInputDriverXInput::ProcessAxisStateChange(int16_t prevState, int16_t newState, unsigned int playerNum, GpGamepadAxis_t gamepadAxis)
{
	if (prevState != newState)
	{
		if (prevState == -32768)
			prevState = -32767;
		if (newState == -32768)
			newState = -32767;

		if (GpVOSEvent *evt = m_properties.m_eventQueue->QueueEvent())
		{
			evt->m_eventType = GpVOSEventTypes::kGamepadInput;
			evt->m_event.m_gamepadInputEvent.m_eventType = GpGamepadInputEventTypes::kAnalogAxisChanged;
			evt->m_event.m_gamepadInputEvent.m_event.m_analogAxisEvent.m_axis = gamepadAxis;
			evt->m_event.m_gamepadInputEvent.m_event.m_analogAxisEvent.m_state = newState;
			evt->m_event.m_gamepadInputEvent.m_event.m_analogAxisEvent.m_player = playerNum;
		}
	}
}

extern "C" __declspec(dllexport) IGpInputDriver *GpDriver_CreateInputDriver_XInput(const GpInputDriverProperties &properties)
{
	return GpInputDriverXInput::Create(properties);
}
