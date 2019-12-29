#pragma once

#include "IGpInputDriver.h"
#include "GpInputDriverProperties.h"
#include "GpVOSEvent.h"
#include "GpWindows.h"

#include <Xinput.h>

class GpInputDriverXInput final : public IGpInputDriver
{
public:
	void ProcessInput() override;
	void Shutdown() override;

	static GpInputDriverXInput *Create(const GpInputDriverProperties &props);

private:
	GpInputDriverXInput(const GpInputDriverProperties &props);
	~GpInputDriverXInput();

	void ProcessControllerStateChange(unsigned int playerNumber, const XINPUT_STATE &prevState, const XINPUT_STATE &newState);
	void ProcessButtonStateChange(DWORD prevState, DWORD newState, DWORD deltaBit, unsigned int playerNum, GpGamepadButton_t gamepadButton);
	void ProcessAxisStateChange(int16_t prevState, int16_t newState, unsigned int playerNum, GpGamepadAxis_t gamepadAxis);

	static int16_t ConvertTriggerValue(uint8_t v);

	GpInputDriverProperties m_properties;
	int m_xUserToPlayerNumber[XUSER_MAX_COUNT];
	bool m_playerNumberIsConnected[XUSER_MAX_COUNT];
	XINPUT_STATE m_controllerStates[XUSER_MAX_COUNT];
};
