#pragma once

struct IGpPrefsHandler;

struct IGpInputDriver
{
	virtual void ProcessInput() = 0;
	virtual void Shutdown() = 0;

	virtual IGpPrefsHandler *GetPrefsHandler() const = 0;
};
