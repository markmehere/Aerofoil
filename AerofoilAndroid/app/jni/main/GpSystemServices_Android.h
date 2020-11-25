#pragma once

#include "IGpSystemServices.h"
#include "GpCoreDefs.h"

class GpSystemServices_Android final : public IGpSystemServices
{
public:
	GpSystemServices_Android();

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

	void FlushTextInputEnabled();

	static GpSystemServices_Android *GetInstance();

private:
	static GpSystemServices_Android ms_instance;

	bool m_textInputEnabled;
};
