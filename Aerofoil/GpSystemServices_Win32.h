#pragma once

#include "IGpSystemServices.h"
#include "GpCoreDefs.h"
#include "GpWindows.h"

#pragma push_macro("CreateMutex")
#ifdef CreateMutex
#undef CreateMutex
#endif

#pragma push_macro("CreateThread")
#ifdef CreateThread
#undef CreateThread
#endif


class GpSystemServices_Win32 final : public IGpSystemServices
{
public:
	GpSystemServices_Win32();

	int64_t GetTime() const override;
	void GetLocalDateTime(unsigned int &year, unsigned int &month, unsigned int &day, unsigned int &hour, unsigned int &minute, unsigned int &second) const override;
	IGpMutex *CreateMutex() override;
	IGpMutex *CreateRecursiveMutex() override;
	void *CreateThread(ThreadFunc_t threadFunc, void *context) override;
	IGpThreadEvent *CreateThreadEvent(bool autoReset, bool startSignaled) override;
	uint64_t GetFreeMemoryCosmetic() const override;
	void Beep() const override;
	bool IsTouchscreen() const override;
	bool IsUsingMouseAsTouch() const override;
	bool IsTextInputObstructive() const override;
	unsigned int GetCPUCount() const override;
	void SetTextInputEnabled(bool isEnabled) override;
	bool IsTextInputEnabled() const override;

	void SetTouchscreenSimulation(bool isTouchscreenSimulation);

	static GpSystemServices_Win32 *GetInstance();

private:
	bool m_isTouchscreenSimulation;

	static GpSystemServices_Win32 ms_instance;
};

#pragma pop_macro("CreateMutex")
#pragma pop_macro("CreateThread")
