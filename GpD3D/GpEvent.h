#pragma once

#include "GpCoreDefs.h"

class GpEvent final
{
public:
	void Wait();
	void WaitMSec(unsigned int msec);
	void Signal();
	void Reset();
	void Destroy();

	static GpEvent *Create(bool autoReset, bool startSignalled);

private:
	explicit GpEvent(void *privateData);
	~GpEvent();

	void *m_PrivateData;
};
