#pragma once

// Display drivers are responsible for timing and calling the game tick function.
class IGpDisplayDriver
{
public:
	virtual ~IGpDisplayDriver() {}

	virtual void Run() = 0;
	virtual void Shutdown() = 0;

	virtual void GetDisplayResolution(unsigned int &width, unsigned int &height) = 0;
};
