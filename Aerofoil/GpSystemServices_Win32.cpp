#include "GpSystemServices_Win32.h"
#include "GpMutex_Win32.h"
#include "GpThreadEvent_Win32.h"
#include "GpWindows.h"
#include "GpAllocator_C.h"

#include "IGpClipboardContents.h"

#include "GpUnicode.h"

#include <assert.h>
#include <vector>

#pragma push_macro("CreateMutex")
#ifdef CreateMutex
#undef CreateMutex
#endif

extern GpWindowsGlobals g_gpWindowsGlobals;

namespace GpSystemServices_Win32_Private
{
	class RefCountedClipboard
	{
	public:
		RefCountedClipboard();

	protected:
		virtual ~RefCountedClipboard();

		void AddRef();
		void DecRef();

		unsigned int m_refCount;
	};

	class TextClipboard : public RefCountedClipboard, public IGpClipboardContentsText
	{
	public:
		TextClipboard(const uint8_t *utf8Text, size_t utf8Size);
		~TextClipboard() override;

		GpClipboardContentsType_t GetContentsType() const override;
		void Destroy() override;
		IGpClipboardContents *Clone() const override;
		const uint8_t *GetBytes() const override;
		size_t GetSize() const override;

	private:
		std::vector<uint8_t> m_utf8Text;
	};


	RefCountedClipboard::RefCountedClipboard()
		: m_refCount(1)
	{
	}

	RefCountedClipboard::~RefCountedClipboard()
	{
	}

	void RefCountedClipboard::AddRef()
	{
		m_refCount++;
	}

	void RefCountedClipboard::DecRef()
	{
		unsigned int rc = --m_refCount;
		if (rc == 0)
			delete this;
	}

	TextClipboard::TextClipboard(const uint8_t *utf8Text, size_t utf8Size)
	{
		m_utf8Text.resize(utf8Size);
		if (utf8Size > 0)
			memcpy(&m_utf8Text[0], utf8Text, utf8Size);
	}

	TextClipboard::~TextClipboard()
	{
	}

	GpClipboardContentsType_t TextClipboard::GetContentsType() const
	{
		return GpClipboardContentsTypes::kText;
	}

	void TextClipboard::Destroy()
	{
		this->DecRef();
	}

	IGpClipboardContents *TextClipboard::Clone() const
	{
		const_cast<TextClipboard*>(this)->AddRef();
		return const_cast<TextClipboard*>(this);
	}

	const uint8_t *TextClipboard::GetBytes() const
	{
		if (m_utf8Text.size() == 0)
			return nullptr;
		return &m_utf8Text[0];
	}

	size_t TextClipboard::GetSize() const
	{
		return m_utf8Text.size();
	}
}

struct GpSystemServices_Win32_ThreadStartParams
{
	GpSystemServices_Win32::ThreadFunc_t m_threadFunc;
	void *m_threadContext;
	IGpThreadEvent *m_threadStartEvent;
};

static DWORD WINAPI StaticStartThread(LPVOID lpThreadParameter)
{
	const GpSystemServices_Win32_ThreadStartParams *threadParams = static_cast<const GpSystemServices_Win32_ThreadStartParams*>(lpThreadParameter);

	GpSystemServices_Win32::ThreadFunc_t threadFunc = threadParams->m_threadFunc;
	void *threadContext = threadParams->m_threadContext;
	IGpThreadEvent *threadStartEvent = threadParams->m_threadStartEvent;

	threadStartEvent->Signal();

	return threadFunc(threadContext);
}

GpSystemServices_Win32::GpSystemServices_Win32()
	: m_isTouchscreenSimulation(false)
	, m_alloc(GpAllocator_C::GetInstance())
{
}

GpSystemServices_Win32::~GpSystemServices_Win32()
{
}

int64_t GpSystemServices_Win32::GetTime() const
{
	SYSTEMTIME epochStart;
	epochStart.wYear = 1904;
	epochStart.wMonth = 1;
	epochStart.wDayOfWeek = 5;
	epochStart.wDay = 1;
	epochStart.wHour = 0;
	epochStart.wMinute = 0;
	epochStart.wSecond = 0;
	epochStart.wMilliseconds = 0;

	FILETIME epochStartFT;
	if (!SystemTimeToFileTime(&epochStart, &epochStartFT))
		return 0;

	FILETIME currentTime;
	GetSystemTimeAsFileTime(&currentTime);

	int64_t epochStart64 = (static_cast<int64_t>(epochStartFT.dwLowDateTime) & 0xffffffff) | (static_cast<int64_t>(epochStartFT.dwHighDateTime) << 32);
	int64_t currentTime64 = (static_cast<int64_t>(currentTime.dwLowDateTime) & 0xffffffff) | (static_cast<int64_t>(currentTime.dwHighDateTime) << 32);

	return (currentTime64 - epochStart64) / 10000000;
}

void GpSystemServices_Win32::GetLocalDateTime(unsigned int &year, unsigned int &month, unsigned int &day, unsigned int &hour, unsigned int &minute, unsigned int &second) const
{
	SYSTEMTIME localTime;
	GetLocalTime(&localTime);

	year = localTime.wYear;
	month = localTime.wMonth;
	day = localTime.wDay;
	hour = localTime.wHour;
	minute = localTime.wMinute;
	second = localTime.wSecond;
}

IGpMutex *GpSystemServices_Win32::CreateMutex()
{
	return GpMutex_Win32::Create(m_alloc);
}

IGpMutex *GpSystemServices_Win32::CreateRecursiveMutex()
{
	return GpMutex_Win32::Create(m_alloc);
}

IGpThreadEvent *GpSystemServices_Win32::CreateThreadEvent(bool autoReset, bool startSignaled)
{
	return GpThreadEvent_Win32::Create(m_alloc, autoReset, startSignaled);
}

void *GpSystemServices_Win32::CreateThread(ThreadFunc_t threadFunc, void *context)
{
	IGpThreadEvent *evt = CreateThreadEvent(true, false);
	if (!evt)
		return nullptr;

	GpSystemServices_Win32_ThreadStartParams startParams;
	startParams.m_threadContext = context;
	startParams.m_threadFunc = threadFunc;
	startParams.m_threadStartEvent = evt;

	HANDLE threadHdl = ::CreateThread(nullptr, 0, StaticStartThread, &startParams, 0, nullptr);
	if (threadHdl == nullptr)
	{
		evt->Destroy();
		return nullptr;
	}

	evt->Wait();
	evt->Destroy();

	return threadHdl;
}

uint64_t GpSystemServices_Win32::GetFreeMemoryCosmetic() const
{
	MEMORYSTATUSEX memStatus;
	memset(&memStatus, 0, sizeof(memStatus));

	memStatus.dwLength = sizeof(memStatus);

	if (!GlobalMemoryStatusEx(&memStatus))
		return 0;

	return memStatus.ullAvailPhys;
}

void GpSystemServices_Win32::Beep() const
{
	MessageBeep(MB_OK);
}

bool GpSystemServices_Win32::IsTouchscreen() const
{
	return m_isTouchscreenSimulation;
}

bool GpSystemServices_Win32::IsUsingMouseAsTouch() const
{
	return m_isTouchscreenSimulation;
}

bool GpSystemServices_Win32::IsTextInputObstructive() const
{
	return false;
}

bool GpSystemServices_Win32::IsFullscreenPreferred() const
{
	return !m_isTouchscreenSimulation;
}

bool GpSystemServices_Win32::IsFullscreenOnStartup() const
{
	return false;
}

bool GpSystemServices_Win32::HasNativeFileManager() const
{
	return false;
}

unsigned int GpSystemServices_Win32::GetCPUCount() const
{
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	return sysInfo.dwNumberOfProcessors;
}

void GpSystemServices_Win32::SetTextInputEnabled(bool isEnabled)
{
	(void)isEnabled;
}

bool GpSystemServices_Win32::IsTextInputEnabled() const
{
	return true;
}

bool GpSystemServices_Win32::AreFontResourcesSeekable() const
{
	return true;
}

IGpClipboardContents *GpSystemServices_Win32::GetClipboardContents() const
{
	IGpClipboardContents *cbObject = nullptr;

	if (IsClipboardFormatAvailable(CF_UNICODETEXT))
	{
		if (OpenClipboard(g_gpWindowsGlobals.m_hwnd))
		{
			HGLOBAL textHandle = GetClipboardData(CF_UNICODETEXT);
			if (textHandle)
			{
				const wchar_t *str = static_cast<const wchar_t*>(GlobalLock(textHandle));
				if (str)
				{
					if (str[0] == 0)
						cbObject = new GpSystemServices_Win32_Private::TextClipboard(nullptr, 0);
					else
					{
						int bytesRequired = WideCharToMultiByte(CP_UTF8, 0, str, -1, nullptr, 0, nullptr, nullptr);

						if (bytesRequired > 0)
						{
							std::vector<char> decodedText;
							decodedText.resize(bytesRequired);
							WideCharToMultiByte(CP_UTF8, 0, str, -1, &decodedText[0], bytesRequired, nullptr, nullptr);

							cbObject = new GpSystemServices_Win32_Private::TextClipboard(reinterpret_cast<const uint8_t*>(&decodedText[0]), decodedText.size() - 1);
						}
					}

					GlobalUnlock(textHandle);
				}
			}
			CloseClipboard();
		}
	}

	return cbObject;
}

void GpSystemServices_Win32::SetClipboardContents(IGpClipboardContents *contents)
{
	if (!contents)
		return;

	if (contents->GetContentsType() == GpClipboardContentsTypes::kText)
	{
		IGpClipboardContentsText *textContents = static_cast<IGpClipboardContentsText*>(contents);

		if (OpenClipboard(g_gpWindowsGlobals.m_hwnd))
		{
			if (EmptyClipboard())
			{
				int wcharsRequired = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(textContents->GetBytes()), textContents->GetSize(), nullptr, 0);

				std::vector<wchar_t> wideChars;

				if (wcharsRequired)
				{
					wideChars.resize(wcharsRequired + 1);
					MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(textContents->GetBytes()), textContents->GetSize(), &wideChars[0], wcharsRequired);
				}
				else
					wideChars.resize(1);

				wideChars[wideChars.size() - 1] = static_cast<wchar_t>(0);

				HGLOBAL textObject = GlobalAlloc(GMEM_MOVEABLE, wideChars.size() * sizeof(wchar_t));
				if (textObject)
				{
					wchar_t *buffer = static_cast<wchar_t*>(GlobalLock(textObject));
					memcpy(buffer, &wideChars[0], wideChars.size() * sizeof(wchar_t));
					GlobalUnlock(textObject);

					SetClipboardData(CF_UNICODETEXT, textObject);
				}
			}

			CloseClipboard();
		}
	}
}


void GpSystemServices_Win32::SetTouchscreenSimulation(bool isTouchscreenSimulation)
{
	m_isTouchscreenSimulation = isTouchscreenSimulation;
}

GpSystemServices_Win32 *GpSystemServices_Win32::GetInstance()
{
	return &ms_instance;
}

GpSystemServices_Win32 GpSystemServices_Win32::ms_instance;


#pragma pop_macro("CreateMutex")
