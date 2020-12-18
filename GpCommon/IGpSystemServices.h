#pragma once

#include <stdint.h>

#ifdef CreateMutex
#error "CreateMutex was macrod"
#endif

#ifdef CreateThread
#error "CreateThread was macrod"
#endif

struct IGpMutex;
struct IGpThreadEvent;
struct IGpClipboardContents;

struct IGpSystemServices
{
public:
	typedef int(*ThreadFunc_t)(void *context);

	virtual int64_t GetTime() const = 0;
	virtual void GetLocalDateTime(unsigned int &year, unsigned int &month, unsigned int &day, unsigned int &hour, unsigned int &minute, unsigned int &second) const = 0;
	virtual IGpMutex *CreateMutex() = 0;
	virtual IGpMutex *CreateRecursiveMutex() = 0;
	virtual void *CreateThread(ThreadFunc_t threadFunc, void *context) = 0;
	virtual IGpThreadEvent *CreateThreadEvent(bool autoReset, bool startSignaled) = 0;
	virtual uint64_t GetFreeMemoryCosmetic() const = 0;	// Returns free memory in bytes, does not have to be accurate
	virtual void Beep() const = 0;
	virtual bool IsTouchscreen() const = 0;
	virtual bool IsUsingMouseAsTouch() const = 0;
	virtual bool IsFullscreenPreferred() const = 0;
	virtual bool IsTextInputObstructive() const = 0;
	virtual unsigned int GetCPUCount() const = 0;
	virtual void SetTextInputEnabled(bool isEnabled) = 0;
	virtual bool IsTextInputEnabled() const = 0;
	virtual bool AreFontResourcesSeekable() const = 0;
	virtual IGpClipboardContents *GetClipboardContents() const = 0;
	virtual void SetClipboardContents(IGpClipboardContents *contents) = 0;
};
