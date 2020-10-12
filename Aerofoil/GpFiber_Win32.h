#pragma once
#include "GpWindows.h"
#include "IGpFiber.h"

class GpFiber_Win32 final : public IGpFiber
{
public:
	void YieldTo(IGpFiber *toFiber) override;
	void YieldToTerminal(IGpFiber *toFiber) override;
	void Destroy() override;

	static IGpFiber *Create(LPVOID fiber);

	LPVOID GetFiber() const;

private:
	explicit GpFiber_Win32(LPVOID fiber);
	~GpFiber_Win32();

	LPVOID m_fiber;
};

inline LPVOID GpFiber_Win32::GetFiber() const
{
	return m_fiber;
}
