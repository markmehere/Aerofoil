#include "WindowManager.h"

#include "DisplayDeviceManager.h"
#include "HostDisplayDriver.h"
#include "PLCore.h"
#include "MemoryManager.h"
#include "QDGraf.h"
#include "QDManager.h"
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

		bool Init(const WindowDef &windowDef, GDevice **device);
		void Resize(int width, int height);

		WindowImpl *GetWindowAbove() const;
		WindowImpl *GetWindowBelow() const;

		void SetWindowAbove(WindowImpl *above);
		void SetWindowBelow(WindowImpl *below);

		bool IsVisible() const;
		void SetVisible(bool visible);

		GDevice **GetDevice() const;

	private:
		WindowImpl *m_windowAbove;
		WindowImpl *m_windowBelow;
		GDevice **m_device;

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
		GDevice **GetWindowDevice(Window *window) override;

		Window *GetPutInFrontSentinel() const override;

		static WindowManagerImpl *GetInstance();

	private:
		void DetachWindow(Window *window);

		WindowImpl *m_windowStackTop;
		WindowImpl *m_windowStackBottom;

		static WindowManagerImpl ms_instance;
		static Window ms_putInFront;
	};

	WindowImpl::WindowImpl()
		: m_windowAbove(nullptr)
		, m_windowBelow(nullptr)
		, m_device(nullptr)
		, m_visible(true)
	{
	}

	WindowImpl::~WindowImpl()
	{
		PL_NotYetImplemented();
	}

	bool WindowImpl::Init(const WindowDef &windowDef, GDevice **device)
	{
		const Rect bounds = windowDef.m_initialRect;

		if (!bounds.IsValid())
			return false;

		const Rect adjustedBounds = Rect::Create(0, 0, bounds.bottom - bounds.top, bounds.right - bounds.left);

		if (int errorCode = m_port.Init(adjustedBounds, (*device)->pixelFormat))
			return false;

		m_device = device;

		return true;
	}

	void WindowImpl::Resize(int width, int height)
	{
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

	GDevice **WindowImpl::GetDevice() const
	{
		return m_device;
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

		GDevice **device = DisplayDeviceManager::GetInstance()->GetMainDevice();

		Rect portRect = windowDef.m_initialRect;
		if (!portRect.IsValid())
			return nullptr;

		WindowImpl *window = new (windowMem) WindowImpl();
		if (!window->Init(windowDef, device))
		{
			window->~WindowImpl();
			MemoryManager::GetInstance()->Release(windowMem);
			return nullptr;
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

	GDevice **WindowManagerImpl::GetWindowDevice(Window *window)
	{
		return static_cast<WindowImpl*>(window)->GetDevice();
	}

	void WindowManagerImpl::ResizeWindow(Window *window, int width, int height)
	{
		static_cast<WindowImpl*>(window)->Resize(width, height);
	}

	void WindowManagerImpl::MoveWindow(Window *window, int x, int y)
	{
		PL_NotYetImplemented_Minor();
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
