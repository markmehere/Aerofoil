#pragma once

struct IGpMutex
{
public:
	virtual void Destroy() = 0;

	virtual void Lock() = 0;
	virtual void Unlock() = 0;
};
