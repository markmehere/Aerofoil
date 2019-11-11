#pragma once
#include "GpWindows.h"
#include "GpFiber.h"

class GpFiber_Win32 final : public GpFiber
{
public:
	explicit GpFiber_Win32(LPVOID fiber);

	void YieldTo() override;
	void Destroy() override;

private:
	LPVOID m_fiber;
};
