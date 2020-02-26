#include "WindowManager.h"

#include "DisplayDeviceManager.h"
#include "HostDisplayDriver.h"
#include "IGpDisplayDriver.h"
#include "IGpDisplayDriverSurface.h"
#include "PLCore.h"
#include "PLEventQueue.h"
#include "PLStandardColors.h"
#include "FontFamily.h"
#include "MemoryManager.h"
#include "MenuManager.h"
#include "QDGraf.h"
#include "QDManager.h"
#include "QDPixMap.h"
#include "PLTimeTaggedVOSEvent.h"
#include "Vec2i.h"
#include "WindowDef.h"

struct GDevice;

namespace PortabilityLayer
{
	class CGrafImpl;
	class WindowImpl;

	struct WindowChromeTheme
	{
		virtual void GetChromePadding(const WindowImpl *window, uint16_t padding[WindowChromeSides::kCount]) const = 0;
		virtual void RenderChrome(WindowImpl *window, DrawSurface *surface, WindowChromeSide_t chromeSide) const = 0;
		virtual bool GetChromeInteractionZone(const WindowImpl *window, const Vec2i &point, RegionID &outRegion) const = 0;
	};

	template<class T>
	struct WindowChromeThemeSingleton : public WindowChromeTheme
	{
	public:
		static T *GetInstance();

	private:
		static T ms_instance;
	};

	class SimpleBoxChromeTheme final : public WindowChromeThemeSingleton<SimpleBoxChromeTheme>
	{
	public:
		void GetChromePadding(const WindowImpl *window, uint16_t padding[WindowChromeSides::kCount]) const override;
		void RenderChrome(WindowImpl *window, DrawSurface *surface, WindowChromeSide_t chromeSide) const override;
		bool GetChromeInteractionZone(const WindowImpl *window, const Vec2i &point, RegionID &outRegion) const override;
	};

	class GenericWindowChromeTheme final : public WindowChromeThemeSingleton<GenericWindowChromeTheme>
	{
	public:
		void GetChromePadding(const WindowImpl *window, uint16_t padding[WindowChromeSides::kCount]) const override;
		void RenderChrome(WindowImpl *window, DrawSurface *surface, WindowChromeSide_t chromeSide) const override;
		bool GetChromeInteractionZone(const WindowImpl *window, const Vec2i &point, RegionID &outRegion) const override;

	private:
		void RenderChromeTop(WindowImpl *window, DrawSurface *surface) const;
		void RenderChromeLeft(WindowImpl *window, DrawSurface *surface) const;
		void RenderChromeBottom(WindowImpl *window, DrawSurface *surface) const;
		void RenderChromeRight(WindowImpl *window, DrawSurface *surface) const;

		void RenderChromeTopMini(WindowImpl *window, DrawSurface *surface) const;
		void RenderChromeLeftMini(WindowImpl *window, DrawSurface *surface) const;
		void RenderChromeBottomMini(WindowImpl *window, DrawSurface *surface) const;
		void RenderChromeRightMini(WindowImpl *window, DrawSurface *surface) const;


		static const int kDarkGray = 85;
		static const int kMidGray = 187;
		static const int kLightGray = 221;
	};

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

		void GetChromePadding(uint16_t padding[WindowChromeSides::kCount]) const;
		void GetChromeDimensions(int width, int height, Rect dimensions[WindowChromeSides::kCount]) const;
		bool GetChromeInteractionZone(const Vec2i &point, RegionID &outRegion) const;

		bool IsBorderless() const;
		uint16_t GetStyleFlags() const;
		const PascalStr<255> &GetTitle() const;

	private:
		WindowImpl *m_windowAbove;
		WindowImpl *m_windowBelow;

		WindowChromeTheme *m_chromeTheme;
		uint16_t m_styleFlags;
		bool m_visible;

		PascalStr<255> m_title;
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
		void DragWindow(Window *window, const Point &startPoint, const Rect &constraintRect) override;

		void RenderFrame(IGpDisplayDriver *displayDriver) override;

		Window *GetPutInFrontSentinel() const override;

		static WindowManagerImpl *GetInstance();

	private:
		void RenderWindow(WindowImpl *window, IGpDisplayDriver *displayDriver);
		void DetachWindow(Window *window);

		WindowImpl *m_windowStackTop;
		WindowImpl *m_windowStackBottom;

		static WindowManagerImpl ms_instance;

		static uint8_t ms_putInFrontSentinel;
	};


	template<class T>
	inline T *WindowChromeThemeSingleton<T>::GetInstance()
	{

		return &ms_instance;
	}

	template<class T>
	T WindowChromeThemeSingleton<T>::ms_instance;

	//---------------------------------------------------------------------------
	void SimpleBoxChromeTheme::GetChromePadding(const WindowImpl *window, uint16_t padding[WindowChromeSides::kCount]) const
	{
		padding[WindowChromeSides::kTop] = 1;
		padding[WindowChromeSides::kBottom] = 1;
		padding[WindowChromeSides::kLeft] = 1;
		padding[WindowChromeSides::kRight] = 1;
	}

	bool SimpleBoxChromeTheme::GetChromeInteractionZone(const WindowImpl *window, const Vec2i &point, RegionID &outRegion) const
	{
		return false;
	}

	void SimpleBoxChromeTheme::RenderChrome(WindowImpl *window, DrawSurface *surface, WindowChromeSide_t chromeSide) const
	{
		surface->SetForeColor(StdColors::Black());
		surface->FillRect((*surface->m_port.GetPixMap())->m_rect);
	}

	//---------------------------------------------------------------------------
	void GenericWindowChromeTheme::GetChromePadding(const WindowImpl *window, uint16_t padding[WindowChromeSides::kCount]) const
	{
		if (window->GetStyleFlags() & WindowStyleFlags::kMiniBar)
		{
			padding[WindowChromeSides::kTop] = 13;
			padding[WindowChromeSides::kBottom] = 1;
			padding[WindowChromeSides::kLeft] = 1;
			padding[WindowChromeSides::kRight] = 1;

			//if (window->GetStyleFlags() & WindowStyleFlags::kResizable)
			//{
			//	padding[WindowChromeSides::kBottom] = 16;
			//	padding[WindowChromeSides::kRight] = 16;
			//}
		}
		else
		{
			padding[WindowChromeSides::kTop] = 22;
			padding[WindowChromeSides::kBottom] = 6;
			padding[WindowChromeSides::kLeft] = 6;
			padding[WindowChromeSides::kRight] = 6;

			//if (window->GetStyleFlags() & WindowStyleFlags::kResizable)
			//{
			//	padding[WindowChromeSides::kBottom] = 21;
			//	padding[WindowChromeSides::kRight] = 21;
			//}
		}
	}

	bool GenericWindowChromeTheme::GetChromeInteractionZone(const WindowImpl *window, const Vec2i &point, RegionID &outRegion) const
	{
		const DrawSurface *surface = window->GetDrawSurface();
		const Rect rect = (*surface->m_port.GetPixMap())->m_rect;

		if (window->GetStyleFlags() & WindowStyleFlags::kMiniBar)
		{
			if (point.m_x >= 0 && point.m_x < rect.Width() && point.m_y < 0 && point.m_y >= -13)
			{
				outRegion = RegionID::inDrag;
				return true;
			}
		}
		else
		{
			if (point.m_x >= 0 && point.m_x < rect.Width() && point.m_y < 0 && point.m_y >= -17)
			{
				outRegion = RegionID::inDrag;
				return true;
			}
		}

		return false;
	}

	void GenericWindowChromeTheme::RenderChrome(WindowImpl *window, DrawSurface *surface, WindowChromeSide_t chromeSide) const
	{
		if (window->GetStyleFlags() & WindowStyleFlags::kMiniBar)
		{
			switch (chromeSide)
			{
			case WindowChromeSides::kTop:
				RenderChromeTopMini(window, surface);
				break;
			case WindowChromeSides::kLeft:
				RenderChromeLeftMini(window, surface);
				break;
			case WindowChromeSides::kBottom:
				RenderChromeBottomMini(window, surface);
				break;
			case WindowChromeSides::kRight:
				RenderChromeRightMini(window, surface);
				break;
			default:
				break;
			}
		}
		else
		{
			switch (chromeSide)
			{
			case WindowChromeSides::kTop:
				RenderChromeTop(window, surface);
				break;
			case WindowChromeSides::kLeft:
				RenderChromeLeft(window, surface);
				break;
			case WindowChromeSides::kBottom:
				RenderChromeBottom(window, surface);
				break;
			case WindowChromeSides::kRight:
				RenderChromeRight(window, surface);
				break;
			default:
				break;
			}
		}
	}

	void GenericWindowChromeTheme::RenderChromeTop(WindowImpl *window, DrawSurface *surface) const
	{
		const Rect rect = (*surface->m_port.GetPixMap())->m_rect;

		surface->SetForeColor(PortabilityLayer::RGBAColor::Create(kMidGray, kMidGray, kMidGray, 255));
		surface->FillRect(rect);

		surface->SetForeColor(StdColors::Black());
		surface->FillRect(Rect::Create(rect.top, rect.left, rect.top + 1, rect.right));
		surface->FillRect(Rect::Create(rect.top, rect.left, rect.bottom, 1));
		surface->FillRect(Rect::Create(rect.top, rect.right - 1, rect.bottom, rect.right));
		surface->FillRect(Rect::Create(rect.bottom - 1, rect.left + 5, rect.bottom, rect.right - 5));

		surface->SetForeColor(PortabilityLayer::RGBAColor::Create(kDarkGray, kDarkGray, kDarkGray, 255));
		surface->FillRect(Rect::Create(rect.bottom - 2, rect.left + 4, rect.bottom - 1, rect.right - 5));
		surface->FillRect(Rect::Create(rect.bottom - 2, rect.left + 4, rect.bottom, rect.left + 5));
		surface->FillRect(Rect::Create(rect.top + 2, rect.right - 2, rect.bottom, rect.right - 1));

		surface->SetForeColor(StdColors::White());
		surface->FillRect(Rect::Create(rect.top + 1, rect.left + 1, rect.bottom, rect.left + 2));
		surface->FillRect(Rect::Create(rect.top + 1, rect.left + 1, rect.top + 2, rect.right - 2));
		surface->FillRect(Rect::Create(rect.bottom - 1, rect.right - 5, rect.bottom, rect.right - 4));

		surface->SetForeColor(StdColors::Black());
		surface->SetSystemFont(12, PortabilityLayer::FontFamilyFlags::FontFamilyFlag_Bold);
		int32_t ascender = surface->MeasureFontAscender();

		const PLPasStr titlePStr = window->GetTitle().ToShortStr();
		size_t titleWidth = surface->MeasureString(titlePStr);

		int32_t titleH = (rect.left + rect.right - static_cast<int32_t>(titleWidth) + 1) / 2;
		int32_t titleV = (rect.top + rect.bottom + ascender + 1) / 2;

		surface->DrawString(Point::Create(titleH, titleV), titlePStr, true);
	}

	void GenericWindowChromeTheme::RenderChromeLeft(WindowImpl *window, DrawSurface *surface) const
	{
		const Rect rect = (*surface->m_port.GetPixMap())->m_rect;

		surface->SetForeColor(StdColors::Black());
		surface->FillRect(Rect::Create(rect.top, rect.right - 6, rect.bottom, rect.right - 5));
		surface->FillRect(Rect::Create(rect.top, rect.right - 1, rect.bottom, rect.right));

		surface->SetForeColor(StdColors::White());
		surface->FillRect(Rect::Create(rect.top, rect.right - 5, rect.bottom, rect.right - 4));

		surface->SetForeColor(PortabilityLayer::RGBAColor::Create(kDarkGray, kDarkGray, kDarkGray, 255));
		surface->FillRect(Rect::Create(rect.top, rect.right - 2, rect.bottom, rect.right - 1));

		surface->SetForeColor(PortabilityLayer::RGBAColor::Create(kMidGray, kMidGray, kMidGray, 255));
		surface->FillRect(Rect::Create(rect.top, rect.right - 4, rect.bottom, rect.right - 2));
	}

	void GenericWindowChromeTheme::RenderChromeBottom(WindowImpl *window, DrawSurface *surface) const
	{
		const Rect rect = (*surface->m_port.GetPixMap())->m_rect;

		surface->SetForeColor(PortabilityLayer::RGBAColor::Create(kMidGray, kMidGray, kMidGray, 255));
		surface->FillRect(Rect::Create(rect.top, rect.left + 1, rect.bottom - 1, rect.left + 5));
		surface->FillRect(Rect::Create(rect.bottom - 4, rect.left + 5, rect.bottom - 2, rect.right - 4));
		surface->FillRect(Rect::Create(rect.top, rect.right - 4, rect.bottom - 2, rect.right - 2));

		surface->SetForeColor(StdColors::Black());
		surface->FillRect(Rect::Create(rect.top, rect.left, rect.bottom, rect.left + 1));
		surface->FillRect(Rect::Create(rect.bottom - 1, rect.left, rect.bottom, rect.right));
		surface->FillRect(Rect::Create(rect.top, rect.right - 1, rect.bottom, rect.right));
		surface->FillRect(Rect::Create(rect.top, rect.left + 5, rect.bottom - 5, rect.left + 6));
		surface->FillRect(Rect::Create(rect.top, rect.right - 6, rect.bottom - 5, rect.right - 5));
		surface->FillRect(Rect::Create(rect.bottom - 6, rect.left + 6, rect.bottom - 5, rect.right - 6));

		surface->SetForeColor(StdColors::White());
		surface->FillRect(Rect::Create(rect.top, rect.right - 5, rect.bottom - 5, rect.right - 4));
		surface->FillRect(Rect::Create(rect.top, rect.left + 1, rect.bottom - 2, rect.left + 2));
		surface->FillRect(Rect::Create(rect.bottom - 5, rect.left + 5, rect.bottom - 4, rect.right - 4));

		surface->SetForeColor(PortabilityLayer::RGBAColor::Create(kDarkGray, kDarkGray, kDarkGray, 255));
		surface->FillRect(Rect::Create(rect.bottom - 2, rect.left + 2, rect.bottom - 1, rect.right - 2));
		surface->FillRect(Rect::Create(rect.top, rect.left + 4, rect.bottom - 5, rect.left + 5));
		surface->FillRect(Rect::Create(rect.top, rect.right - 2, rect.bottom - 1, rect.right - 1));
	}

	void GenericWindowChromeTheme::RenderChromeRight(WindowImpl *window, DrawSurface *surface) const
	{
		const Rect rect = (*surface->m_port.GetPixMap())->m_rect;

		surface->SetForeColor(StdColors::Black());
		surface->FillRect(Rect::Create(rect.top, rect.right - 6, rect.bottom, rect.right - 5));
		surface->FillRect(Rect::Create(rect.top, rect.right - 1, rect.bottom, rect.right));

		surface->SetForeColor(StdColors::White());
		surface->FillRect(Rect::Create(rect.top, rect.right - 5, rect.bottom, rect.right - 4));

		surface->SetForeColor(PortabilityLayer::RGBAColor::Create(kDarkGray, kDarkGray, kDarkGray, 255));
		surface->FillRect(Rect::Create(rect.top, rect.right - 2, rect.bottom, rect.right - 1));

		surface->SetForeColor(PortabilityLayer::RGBAColor::Create(kMidGray, kMidGray, kMidGray, 255));
		surface->FillRect(Rect::Create(rect.top, rect.right - 4, rect.bottom, rect.right - 2));
	}

	void GenericWindowChromeTheme::RenderChromeTopMini(WindowImpl *window, DrawSurface *surface) const
	{
		const Rect rect = (*surface->m_port.GetPixMap())->m_rect;

		surface->SetForeColor(PortabilityLayer::RGBAColor::Create(kLightGray, kLightGray, kLightGray, 255));
		surface->FillRect(rect);

		surface->SetForeColor(StdColors::Black());
		surface->FillRect(Rect::Create(rect.top, rect.left, rect.top + 1, rect.right));
		surface->FillRect(Rect::Create(rect.top, rect.left, rect.bottom, 1));
		surface->FillRect(Rect::Create(rect.top, rect.right - 1, rect.bottom, rect.right));
		surface->FillRect(Rect::Create(rect.bottom - 1, rect.left + 1, rect.bottom, rect.right - 1));

		surface->SetForeColor(PortabilityLayer::RGBAColor::Create(kDarkGray, kDarkGray, kDarkGray, 255));
		surface->FillRect(Rect::Create(rect.bottom - 2, rect.left + 2, rect.bottom - 1, rect.right - 2));
		surface->FillRect(Rect::Create(rect.top + 2, rect.right - 2, rect.bottom - 1, rect.right - 1));

		surface->SetForeColor(StdColors::White());
		surface->FillRect(Rect::Create(rect.top + 1, rect.left + 1, rect.bottom - 2, rect.left + 2));
		surface->FillRect(Rect::Create(rect.top + 1, rect.left + 1, rect.top + 2, rect.right - 2));

		surface->SetForeColor(StdColors::Black());
		surface->SetApplicationFont(10, PortabilityLayer::FontFamilyFlags::FontFamilyFlag_Bold);
		int32_t ascender = surface->MeasureFontAscender();

		const PLPasStr titlePStr = window->GetTitle().ToShortStr();
		size_t titleWidth = surface->MeasureString(titlePStr);

		int32_t titleH = (rect.left + rect.right - static_cast<int32_t>(titleWidth) + 1) / 2;
		int32_t titleV = (rect.top + rect.bottom + ascender + 1) / 2;

		surface->DrawString(Point::Create(titleH, titleV), titlePStr, true);
	}

	void GenericWindowChromeTheme::RenderChromeLeftMini(WindowImpl *window, DrawSurface *surface) const
	{
		const Rect rect = (*surface->m_port.GetPixMap())->m_rect;

		surface->SetForeColor(StdColors::Black());
		surface->FillRect(rect);
	}

	void GenericWindowChromeTheme::RenderChromeBottomMini(WindowImpl *window, DrawSurface *surface) const
	{
		const Rect rect = (*surface->m_port.GetPixMap())->m_rect;

		surface->SetForeColor(StdColors::Black());
		surface->FillRect(rect);
	}

	void GenericWindowChromeTheme::RenderChromeRightMini(WindowImpl *window, DrawSurface *surface) const
	{
		const Rect rect = (*surface->m_port.GetPixMap())->m_rect;

		surface->SetForeColor(StdColors::Black());
		surface->FillRect(rect);
	}

	//---------------------------------------------------------------------------
	WindowImpl::WindowImpl()
		: m_windowAbove(nullptr)
		, m_windowBelow(nullptr)
		, m_chromeTheme(nullptr)
		, m_visible(true)
		, m_styleFlags(0)
	{
	}

	WindowImpl::~WindowImpl()
	{
		assert(m_windowAbove == nullptr && m_windowBelow == nullptr);
	}

	bool WindowImpl::Init(const WindowDef &windowDef)
	{
		const Rect bounds = windowDef.m_initialRect;

		if (!bounds.IsValid())
			return false;

		m_styleFlags = windowDef.m_styleFlags;

		const Rect adjustedBounds = Rect::Create(0, 0, bounds.Height(), bounds.Width());

		GpPixelFormat_t pixelFormat = PortabilityLayer::DisplayDeviceManager::GetInstance()->GetPixelFormat();

		if (int errorCode = m_surface.Init(adjustedBounds, pixelFormat))
			return false;

		m_title.Set(windowDef.m_title[0], reinterpret_cast<const char*>(windowDef.m_title + 1));

		// Resolve chrome
		if (!IsBorderless())
		{
			m_chromeTheme = SimpleBoxChromeTheme::GetInstance();

			if (m_styleFlags & WindowStyleFlags::kTitleBar)
				m_chromeTheme = GenericWindowChromeTheme::GetInstance();

			Rect chromeBounds[WindowChromeSides::kCount];
			GetChromeDimensions(bounds.Width(), bounds.Height(), chromeBounds);

			for (int chromeSide = 0; chromeSide < WindowChromeSides::kCount; chromeSide++)
			{
				if (int errorCode = m_chromeSurfaces[chromeSide].Init(chromeBounds[chromeSide], pixelFormat))
					return false;

				m_chromeTheme->RenderChrome(this, m_chromeSurfaces + chromeSide, static_cast<WindowChromeSide_t>(chromeSide));
			}
		}

		return true;
	}

	bool WindowImpl::Resize(int width, int height)
	{
		Rect rect = m_surface.m_port.GetRect();
		rect.right = rect.left + width;
		rect.bottom = rect.top + height;

		if (!m_surface.Resize(rect))
			return false;

		if (!IsBorderless())
		{
			Rect chromeDimensions[WindowChromeSides::kCount];

			GetChromeDimensions(width, height, chromeDimensions);

			bool resized = true;
			for (int i = 0; i < WindowChromeSides::kCount; i++)
			{
				if (!m_chromeSurfaces[i].Resize(chromeDimensions[i]))
					return false;

				m_chromeTheme->RenderChrome(this, m_chromeSurfaces + i, static_cast<WindowChromeSide_t>(i));
			}
		}

		return true;
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

	void WindowImpl::GetChromePadding(uint16_t padding[WindowChromeSides::kCount]) const
	{
		return m_chromeTheme->GetChromePadding(this, padding);
	}

	void WindowImpl::GetChromeDimensions(int width, int height, Rect dimensions[WindowChromeSides::kCount]) const
	{
		uint16_t padding[WindowChromeSides::kCount];
		GetChromePadding(padding);

		int32_t topAndBottomWidth = width + padding[WindowChromeSides::kLeft] + padding[WindowChromeSides::kRight];
		int32_t leftAndRightHeight = height;

		dimensions[WindowChromeSides::kTop] = Rect::Create(0, 0, padding[WindowChromeSides::kTop], topAndBottomWidth);
		dimensions[WindowChromeSides::kBottom] = Rect::Create(0, 0, padding[WindowChromeSides::kBottom], topAndBottomWidth);
		dimensions[WindowChromeSides::kLeft] = Rect::Create(0, 0, leftAndRightHeight, padding[WindowChromeSides::kLeft]);
		dimensions[WindowChromeSides::kRight] = Rect::Create(0, 0, leftAndRightHeight, padding[WindowChromeSides::kRight]);
	}

	bool WindowImpl::GetChromeInteractionZone(const Vec2i &point, RegionID &outRegion) const
	{
		return m_chromeTheme->GetChromeInteractionZone(this, point, outRegion);
	}

	bool WindowImpl::IsBorderless() const
	{
		return (m_styleFlags & WindowStyleFlags::kBorderless) != 0;
	}

	uint16_t WindowImpl::GetStyleFlags() const
	{
		return m_styleFlags;
	}

	const PascalStr<255> &WindowImpl::GetTitle() const
	{
		return m_title;
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
			const Rect windowRect = window->m_surface.m_port.GetRect();

			const int32_t localX = point.h - window->m_wmX;
			const int32_t localY = point.v - window->m_wmY;

			RegionID chromeInteractionZone = inContent;
			if (window->GetChromeInteractionZone(Vec2i(localX, localY), chromeInteractionZone))
			{
				*outRegion = chromeInteractionZone;
				*outWindow = window;
				return;
			}

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

		if (PortabilityLayer::QDManager::GetInstance()->GetPort() == &windowImpl->m_surface.m_port)
			PortabilityLayer::QDManager::GetInstance()->SetPort(nullptr);

		windowImpl->~WindowImpl();
		PortabilityLayer::MemoryManager::GetInstance()->Release(windowImpl);
	}

	void WindowManagerImpl::DragWindow(Window *window, const Point &startPointRef, const Rect &constraintRect)
	{
		int32_t baseX = startPointRef.h;
		int32_t baseY = startPointRef.v;

		for (;;)
		{
			TimeTaggedVOSEvent evt;
			if (WaitForEvent(&evt, 1))
			{
				if (evt.m_vosEvent.m_eventType == GpVOSEventTypes::kMouseInput)
				{
					const GpMouseInputEvent &mouseEvent = evt.m_vosEvent.m_event.m_mouseInputEvent;
					int32_t x = mouseEvent.m_x;
					int32_t y = mouseEvent.m_y;

					if (x < constraintRect.left)
						x = constraintRect.left;
					if (x >= constraintRect.right)
						x = constraintRect.right - 1;
					if (y < constraintRect.top)
						y = constraintRect.top;
					if (y >= constraintRect.bottom)
						y = constraintRect.bottom - 1;

					window->m_wmX += x - baseX;
					window->m_wmY += y - baseY;

					baseX = x;
					baseY = y;

					if (mouseEvent.m_eventType == GpMouseEventTypes::kUp)
						break;
				}
			}
		}
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
		return reinterpret_cast<Window*>(&ms_putInFrontSentinel);
	}

	void WindowManagerImpl::RenderWindow(WindowImpl *window, IGpDisplayDriver *displayDriver)
	{
		if (!window->IsVisible())
			return;

		DrawSurface &graf = window->m_surface;

		graf.PushToDDSurface(displayDriver);

		const PixMap *pixMap = *graf.m_port.GetPixMap();
		const uint16_t width = pixMap->m_rect.Width();
		const uint16_t height = pixMap->m_rect.Height();
		displayDriver->DrawSurface(graf.m_ddSurface, window->m_wmX, window->m_wmY, width, height);


		if (!window->IsBorderless())
		{
			uint16_t chromePadding[WindowChromeSides::kCount];
			window->GetChromePadding(chromePadding);

			Vec2i chromeOrigins[WindowChromeSides::kCount];
			chromeOrigins[WindowChromeSides::kTop] = Vec2i(window->m_wmX - chromePadding[WindowChromeSides::kLeft], window->m_wmY - chromePadding[WindowChromeSides::kTop]);
			chromeOrigins[WindowChromeSides::kLeft] = Vec2i(window->m_wmX - chromePadding[WindowChromeSides::kLeft], window->m_wmY);
			chromeOrigins[WindowChromeSides::kRight] = Vec2i(window->m_wmX + width, window->m_wmY);
			chromeOrigins[WindowChromeSides::kBottom] = Vec2i(window->m_wmX - chromePadding[WindowChromeSides::kLeft], window->m_wmY + height);

			Vec2i chromeDimensions[WindowChromeSides::kCount];
			chromeDimensions[WindowChromeSides::kTop] = Vec2i(chromePadding[WindowChromeSides::kLeft] + chromePadding[WindowChromeSides::kRight] + width, chromePadding[WindowChromeSides::kTop]);
			chromeDimensions[WindowChromeSides::kLeft] = Vec2i(chromePadding[WindowChromeSides::kLeft], height);
			chromeDimensions[WindowChromeSides::kRight] = Vec2i(chromePadding[WindowChromeSides::kRight], height);
			chromeDimensions[WindowChromeSides::kBottom] = Vec2i(chromePadding[WindowChromeSides::kLeft] + chromePadding[WindowChromeSides::kRight] + width, chromePadding[WindowChromeSides::kBottom]);

			for (int i = 0; i < WindowChromeSides::kCount; i++)
			{
				DrawSurface *chromeSurface = window->GetChromeSurface(static_cast<WindowChromeSide_t>(i));

				chromeSurface->PushToDDSurface(displayDriver);

				displayDriver->DrawSurface(chromeSurface->m_ddSurface, chromeOrigins[i].m_x, chromeOrigins[i].m_y, chromeDimensions[i].m_x, chromeDimensions[i].m_y);
			}
		}
	}

	WindowManagerImpl *WindowManagerImpl::GetInstance()
	{
		return &ms_instance;
	}

	WindowManagerImpl WindowManagerImpl::ms_instance;
	uint8_t WindowManagerImpl::ms_putInFrontSentinel;

	WindowManager *WindowManager::GetInstance()
	{
		return WindowManagerImpl::GetInstance();
	}
}
