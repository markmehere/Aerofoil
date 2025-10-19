#pragma once

#include "GpSystemServices_POSIX.h"
#include "GpCoreDefs.h"
#include <jni.h>

class GpSystemServices_Android final : public GpSystemServices_POSIX
{
public:
	GpSystemServices_Android();

	void *CreateThread(ThreadFunc_t threadFunc, void *context) override;
	bool Beep() const override;
	bool IsTouchscreen() const override;
	bool IsUsingMouseAsTouch() const override;
	bool IsTextInputObstructive() const override;
	bool IsFullscreenPreferred() const override;
	bool IsFullscreenOnStartup() const override;
	bool HasNativeFileManager() const override;
	GpOperatingSystem_t GetOperatingSystem() const override;
	GpOperatingSystemFlavor_t GetOperatingSystemFlavor() const override;
	unsigned int GetCPUCount() const override;
	void SetTextInputEnabled(bool isEnabled) override;
	bool IsTextInputEnabled() const override;
	bool AreFontResourcesSeekable() const override;
	IGpClipboardContents *GetClipboardContents() const override;
	void SetClipboardContents(IGpClipboardContents *contents) override;

	void FlushTextInputEnabled();

	static GpSystemServices_Android *GetInstance();
	void InitJNI();

private:
	static GpSystemServices_Android ms_instance;

	jobject m_activity;
	jmethodID m_showTextInputMID;
	jmethodID m_hideTextInputMID;
	bool m_textInputEnabled;
};
