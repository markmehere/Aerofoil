#pragma once

struct IGpInputDriver
{
	virtual void ProcessInput() = 0;
	virtual void Shutdown() = 0;
};
