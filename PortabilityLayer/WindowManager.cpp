#include "WindowManager.h"

#include "DisplayDeviceManager.h"
#include "HostDisplayDriver.h"
#include "IGpDisplayDriver.h"
#include "IGpDisplayDriverSurface.h"
#include "PLCore.h"
#include "PLEventQueue.h"
#include "MemoryManager.h"
#include "MenuManager.h"
#include "QDGraf.h"
#include "QDManager.h"
#include "QDPixMap.h"
#include "Vec2i.h"
#include "WindowDef.h"

struct GDevice;

namespace PortabilityLayer
{
	class CGrafImpl;

	class WindowImpl final : public Window
	{
	public:
		WindowImpl();
		~WindowImpl();

		bool Init(const WindowDef &windowDef);
		bool Resize(int width, int height);

		WindowImpl *GetWindowAbove() const;
		WindowImpl *GetWindowBelow() const;

		void SetWindowAbove(WindowImpl *above);
		void SetWindowBelow(WindowImpl *below);

		bool IsVisible() const;
		void SetVisible(bool visible);

	private:
		WindowImpl *m_windowAbove;
		WindowImpl *m_windowBelow;

		bool m_visible;
	};

	class WindowManagerImpl final : public WindowManager
	{
	public:
		WindowManagerImpl();

		Window *CreateWindow(const WindowDef &windowDef) override;
		void ResizeWindow(Window *window, int width, int height) override;
		void MoveWindow(Window *window, int x, int y) override;
		void PutWindowBehind(Window *window, Window *otherWindow) override;
		void ShowWindow(Window *window) override;
		void HideWindow(Window *window) override;
		void FindWindow(const Point &point, Window **outWindow, short *outRegion) const override;
		void DestroyWindow(Window *window) override;

		void RenderFrame(IGpDisplayDriver *displayDriver) override;

		Window *GetPutInFrontSentinel() const override;

		static WindowManagerImpl *GetInstance();

	private:
		void RenderWindow(WindowImpl *window, IGpDisplayDriver *displayDriver);
		void DetachWindow(Window *window);

		WindowImpl *m_windowStackTop;
		WindowImpl *m_windowStackBottom;

		static WindowManagerImpl ms_instance;
		static Window ms_putInFront;
	};

	WindowImpl::WindowImpl()
		: m_windowAbove(nullptr)
		, m_windowBelow(nullptr)
		, m_visible(true)
	{
	}

	WindowImpl::~WindowImpl()
	{
		PL_NotYetImplemented();
	}

	bool WindowImpl::Init(const WindowDef &windowDef)
	{
		const Rect bounds = windowDef.m_initialRect;

		if (!bounds.IsValid())
			return false;

		const Rect adjustedBounds = Rect::Create(0, 0, bounds.bottom - bounds.top, bounds.right - bounds.left);

		GpPixelFormat_t pixelFormat = PortabilityLayer::DisplayDeviceManager::GetInstance()->GetPixelFormat();

		if (int errorCode = m_graf.Init(adjustedBounds, pixelFormat))
			return false;

		return true;
	}

	bool WindowImpl::Resize(int width, int height)
	{
		Rect rect = m_graf.m_port.GetRect();
		rect.right = rect.left + width;
		rect.bottom = rect.top + height;

		return m_graf.Resize(rect);
	}

	WindowImpl *WindowImpl::GetWindowAbove() const
	{
		return m_windowAbove;
	}

	WindowImpl *WindowImpl::GetWindowBelow() const
	{
		return m_windowBelow;
	}

	void WindowImpl::SetWindowAbove(WindowImpl *above)
	{
		m_windowAbove = above;
	}

	void WindowImpl::SetWindowBelow(WindowImpl *below)
	{
		m_windowBelow = below;
	}

	bool WindowImpl::IsVisible() const
	{
		return m_visible;
	}

	void WindowImpl::SetVisible(bool visible)
	{
		m_visible = visible;
	}

	WindowManagerImpl::WindowManagerImpl()
		: m_windowStackTop(nullptr)
		, m_windowStackBottom(nullptr)
	{
	}

	Window *WindowManagerImpl::CreateWindow(const WindowDef &windowDef)
	{
		void *windowMem = MemoryManager::GetInstance()->Alloc(sizeof(WindowImpl));
		if (!windowMem)
			return nullptr;

		Rect portRect = windowDef.m_initialRect;
		if (!portRect.IsValid())
			return nullptr;

		WindowImpl *window = new (windowMem) WindowImpl();
		if (!window->Init(windowDef))
		{
			window->~WindowImpl();
			MemoryManager::GetInstance()->Release(windowMem);
			return nullptr;
		}

		if (EventRecord *evt = PortabilityLayer::EventQueue::GetInstance()->Enqueue())
		{
			evt->what = updateEvt;
			evt->message = reinterpret_cast<intptr_t>(static_cast<Window*>(window));
		}

		return window;
	}

	void WindowManagerImpl::PutWindowBehind(Window *windowBase, Window *behind)
	{
		WindowImpl *window = static_cast<WindowImpl*>(windowBase);

		if (window->GetWindowAbove() != behind)
			DetachWindow(window);

		if (behind == GetPutInFrontSentinel())
		{
			if (m_windowStackTop)
			{
				m_windowStackTop->SetWindowAbove(window);
				window->SetWindowBelow(m_windowStackTop);
				m_windowStackTop = window;
			}
			else
			{
				m_windowStackTop = m_windowStackBottom = window;
			}
		}
		else
		{
			WindowImpl *windowAbove = static_cast<WindowImpl*>(behind);
			WindowImpl *windowBelow = windowAbove->GetWindowBelow();

			window->SetWindowAbove(windowAbove);
			window->SetWindowBelow(windowBelow);

			if (windowBelow)
				windowBelow->SetWindowAbove(window);
			else
				m_windowStackBottom = window;

			windowAbove->SetWindowBelow(window);
		}
	}

	void WindowManagerImpl::ShowWindow(Window *window)
	{
		static_cast<WindowImpl*>(window)->SetVisible(true);
	}

	void WindowManagerImpl::HideWindow(Window *window)
	{
		WindowImpl *impl = static_cast<WindowImpl*>(window);
		impl->SetVisible(false);

		// Per spec, hiding a window brings the window below it to the front
		if (m_windowStackTop == impl)
		{
			if (WindowImpl *below = impl->GetWindowBelow())
				BringToFront(below);
		}
	}

	void WindowManagerImpl::FindWindow(const Point &point, Window **outWindow, short *outRegion) const
	{
		// outRegion = One of:
		/*
		inMenuBar,
		inContent,
		inDrag,
		inGrow,
		inGoAway,
		inZoomIn,
		inZoomOut,
		*/

		if (PortabilityLayer::MenuManager::GetInstance()->IsPointInMenuBar(PortabilityLayer::Vec2i(point.h, point.v)))
		{
			if (outWindow)
				*outWindow = nullptr;

			if (outRegion)
				*outRegion = inMenuBar;

			return;
		}

		WindowImpl *window = m_windowStackTop;
		while (window)
		{
			const Rect windowRect = window->m_graf.m_port.GetRect();

			const int32_t localX = point.h - window->m_wmX;
			const int32_t localY = point.v - window->m_wmY;

			if (localX >= 0 && localY >= 0 && localX < windowRect.right && localY < windowRect.bottom)
			{
				if (outWindow)
					*outWindow = window;

				if (outRegion)
					*outRegion = inContent;

				return;
			}

			window = window->GetWindowBelow();
		}

		if (outWindow)
			*outWindow = nullptr;

		if (outRegion)
			*outRegion = 0;
	}

	void WindowManagerImpl::DestroyWindow(Window *window)
	{
		WindowImpl *windowImpl = static_cast<WindowImpl*>(window);

		DetachWindow(window);

		windowImpl->~WindowImpl();
		PortabilityLayer::MemoryManager::GetInstance()->Release(windowImpl);
	}

	void WindowManagerImpl::RenderFrame(IGpDisplayDriver *displayDriver)
	{
		PortabilityLayer::DisplayDeviceManager *dd = PortabilityLayer::DisplayDeviceManager::GetInstance();

		dd->SyncPalette(displayDriver);

		WindowImpl *window = m_windowStackBottom;
		while (window)
		{
			RenderWindow(window, displayDriver);
			window = window->GetWindowAbove();
		}
	}

	void WindowManagerImpl::ResizeWindow(Window *window, int width, int height)
	{
		static_cast<WindowImpl*>(window)->Resize(width, height);

		if (EventRecord *evt = PortabilityLayer::EventQueue::GetInstance()->Enqueue())
		{
			evt->what = updateEvt;
			evt->message = reinterpret_cast<intptr_t>(window);
		}
	}

	void WindowManagerImpl::MoveWindow(Window *window, int x, int y)
	{
		window->m_wmX = x;
		window->m_wmY = y;
	}

	void WindowManagerImpl::DetachWindow(Window *window)
	{
		if (m_windowStackBottom == window)
			m_windowStackBottom = m_windowStackBottom->GetWindowAbove();
		if (m_windowStackTop == window)
			m_windowStackTop = m_windowStackTop->GetWindowBelow();

		WindowImpl *impl = static_cast<WindowImpl*>(window);

		if (WindowImpl *below = impl->GetWindowBelow())
			below->SetWindowAbove(impl->GetWindowAbove());

		if (WindowImpl *above = impl->GetWindowAbove())
			above->SetWindowBelow(impl->GetWindowBelow());

		impl->SetWindowAbove(nullptr);
		impl->SetWindowBelow(nullptr);
	}

	Window *WindowManagerImpl::GetPutInFrontSentinel() const
	{
		return &ms_putInFront;
	}

	void WindowManagerImpl::RenderWindow(WindowImpl *window, IGpDisplayDriver *displayDriver)
	{
		DrawSurface &graf = window->m_graf;

		graf.PushToDDSurface(displayDriver);

		const PixMap *pixMap = *graf.m_port.GetPixMap();
		const size_t width = pixMap->m_rect.right - pixMap->m_rect.left;
		const size_t height = pixMap->m_rect.bottom - pixMap->m_rect.top;
		displayDriver->DrawSurface(graf.m_ddSurface, window->m_wmX, window->m_wmY, width, height);
	}

	WindowManagerImpl *WindowManagerImpl::GetInstance()
	{
		return &ms_instance;
	}

	WindowManagerImpl WindowManagerImpl::ms_instance;
	Window WindowManagerImpl::ms_putInFront;

	WindowManager *WindowManager::GetInstance()
	{
		return WindowManagerImpl::GetInstance();
	}
}
