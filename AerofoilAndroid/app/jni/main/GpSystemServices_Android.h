#pragma once

#include "GpSystemServices_POSIX.h"
#include "GpCoreDefs.h"

class GpSystemServices_Android final : public GpSystemServices_POSIX
{
public:
	GpSystemServices_Android();

	void *CreateThread(ThreadFunc_t threadFunc, void *context) override;
	void Beep() const override;
	bool IsTouchscreen() const override;
	bool IsUsingMouseAsTouch() const override;
	bool IsTextInputObstructive() const override;
	bool IsFullscreenPreferred() const override;
	bool IsFullscreenOnStartup() const override;
	unsigned int GetCPUCount() const override;
	void SetTextInputEnabled(bool isEnabled) override;
	bool IsTextInputEnabled() const override;
	bool AreFontResourcesSeekable() const override;
	IGpClipboardContents *GetClipboardContents() const override;
	void SetClipboardContents(IGpClipboardContents *contents) override;

	void FlushTextInputEnabled();

	static GpSystemServices_Android *GetInstance();

private:
	static GpSystemServices_Android ms_instance;

	bool m_textInputEnabled;
};
