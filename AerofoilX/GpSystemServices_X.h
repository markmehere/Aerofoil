#pragma once

#include "GpSystemServices_POSIX.h"
#include "GpCoreDefs.h"

struct IGpClipboardContents;

class GpSystemServices_X final : public GpSystemServices_POSIX
{
public:
	GpSystemServices_X();
	~GpSystemServices_X();

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

	static GpSystemServices_X *GetInstance();

private:
	static GpSystemServices_X ms_instance;

	IGpClipboardContents *m_clipboardContents;
	bool m_textInputEnabled;
};
