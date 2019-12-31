#pragma once

struct Window;
struct DrawSurface;
struct GDevice;
struct IGpDisplayDriver;
struct Point;
struct Window;

namespace PortabilityLayer
{
	struct WindowDef;

	class WindowManager
	{
	public:
		virtual Window *GetPutInFrontSentinel() const = 0;
		virtual Window *CreateWindow(const WindowDef &windowDef) = 0;
		virtual void ResizeWindow(Window *window, int width, int height) = 0;
		virtual void MoveWindow(Window *window, int x, int y) = 0;
		virtual void PutWindowBehind(Window *window, Window *otherWindow) = 0;
		virtual void ShowWindow(Window *window) = 0;
		virtual void HideWindow(Window *window) = 0;
		virtual void FindWindow(const Point &point, Window **outWindow, short *outRegion) const = 0;
		virtual void DestroyWindow(Window *window) = 0;

		virtual void RenderFrame(IGpDisplayDriver *displayDriver) = 0;

		static WindowManager *GetInstance();
	};
}
