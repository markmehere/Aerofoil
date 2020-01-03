#pragma once
#include "GpWindows.h"
#include "IGpFiber.h"

class GpFiber_Win32 final : public IGpFiber
{
public:
	void YieldTo() override;
	void Destroy() override;

	static IGpFiber *Create(LPVOID fiber);

private:
	explicit GpFiber_Win32(LPVOID fiber);
	~GpFiber_Win32();

	LPVOID m_fiber;
};
