#include "GpSystemServices_Android.h"

#include "IGpThreadEvent.h"
#include "SDL.h"

#include <time.h>
#include <unistd.h>

struct GpSystemServices_Android_ThreadStartParams
{
	GpSystemServices_Android::ThreadFunc_t m_threadFunc;
	void *m_threadContext;
	IGpThreadEvent *m_threadStartEvent;
};

static int SDLCALL StaticStartThread(void *lpThreadParameter)
{
	const GpSystemServices_Android_ThreadStartParams *threadParams = static_cast<const GpSystemServices_Android_ThreadStartParams*>(lpThreadParameter);

	GpSystemServices_Android::ThreadFunc_t threadFunc = threadParams->m_threadFunc;
	void *threadContext = threadParams->m_threadContext;
	IGpThreadEvent *threadStartEvent = threadParams->m_threadStartEvent;

	threadStartEvent->Signal();

	return threadFunc(threadContext);
}

GpSystemServices_Android::GpSystemServices_Android()
	: m_textInputEnabled(false)
{
}

void GpSystemServices_Android::InitJNI()
{
	JNIEnv *jni = static_cast<JNIEnv *>(SDL_AndroidGetJNIEnv());

	jobject activityLR = static_cast<jobject>(SDL_AndroidGetActivity());
	jclass activityClassLR = static_cast<jclass>(jni->GetObjectClass(activityLR));

	m_showTextInputMID = jni->GetMethodID(activityClassLR, "showTextInput", "()V");
	m_hideTextInputMID = jni->GetMethodID(activityClassLR, "hideTextInput", "()V");
	m_activity = jni->NewGlobalRef(activityLR);

	jni->DeleteLocalRef(activityLR);
	jni->DeleteLocalRef(activityClassLR);
}

void *GpSystemServices_Android::CreateThread(ThreadFunc_t threadFunc, void *context)
{
	IGpThreadEvent *evt = CreateThreadEvent(true, false);
	if (!evt)
		return nullptr;

	GpSystemServices_Android_ThreadStartParams startParams;
	startParams.m_threadContext = context;
	startParams.m_threadFunc = threadFunc;
	startParams.m_threadStartEvent = evt;

	SDL_Thread *thread = SDL_CreateThread(StaticStartThread, "WorkerThread", &startParams);
	if (thread == nullptr)
	{
		evt->Destroy();
		return nullptr;
	}

	evt->Wait();
	evt->Destroy();

	return thread;
}

bool GpSystemServices_Android::Beep() const
{
	return false;
}

bool GpSystemServices_Android::IsTouchscreen() const
{
	return true;
}

bool GpSystemServices_Android::IsUsingMouseAsTouch() const
{
	return false;
}

bool GpSystemServices_Android::IsTextInputObstructive() const
{
	return true;
}

bool GpSystemServices_Android::IsFullscreenPreferred() const
{
	return true;
}

bool GpSystemServices_Android::IsFullscreenOnStartup() const
{
	return true;
}

bool GpSystemServices_Android::HasNativeFileManager() const
{
	return false;
}

GpOperatingSystem_t GpSystemServices_Android::GetOperatingSystem() const
{
	return GpOperatingSystems::kAndroid;
}

GpOperatingSystemFlavor_t GpSystemServices_Android::GetOperatingSystemFlavor() const
{
	return GpOperatingSystemFlavors::kGeneric;
}

unsigned int GpSystemServices_Android::GetCPUCount() const
{
	return SDL_GetCPUCount();
}

void GpSystemServices_Android::SetTextInputEnabled(bool isEnabled)
{
	if (!m_textInputEnabled && isEnabled) {
		JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
		env->CallVoidMethod(m_activity, m_showTextInputMID);
	}
	else if (m_textInputEnabled && !isEnabled) {
		JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
		env->CallVoidMethod(m_activity, m_hideTextInputMID);
	}
	m_textInputEnabled = isEnabled;
}

bool GpSystemServices_Android::IsTextInputEnabled() const
{
	return m_textInputEnabled;
}

bool GpSystemServices_Android::AreFontResourcesSeekable() const
{
	return false;
}

IGpClipboardContents *GpSystemServices_Android::GetClipboardContents() const
{
	return nullptr;
}

void GpSystemServices_Android::SetClipboardContents(IGpClipboardContents *contents)
{
}

GpSystemServices_Android *GpSystemServices_Android::GetInstance()
{
	return &ms_instance;
}

GpSystemServices_Android GpSystemServices_Android::ms_instance;
