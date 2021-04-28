#pragma once

struct IGpAudioBuffer
{
public:
	virtual void AddRef() = 0;
	virtual void Release() = 0;
};
