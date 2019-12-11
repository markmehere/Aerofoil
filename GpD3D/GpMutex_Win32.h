#pragma once

#include "HostMutex.h"

#include "GpWindows.h"

class GpMutex_Win32 final : public PortabilityLayer::HostMutex
{
public:
	void Destroy() override;
	void Lock() override;
	void Unlock() override;

	static GpMutex_Win32 *Create();

private:
	const GpMutex_Win32();
	~GpMutex_Win32();

	CRITICAL_SECTION m_critSection;
};
