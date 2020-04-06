#pragma once

#include <stdint.h>

struct Window;
struct DrawSurface;
struct GDevice;
struct IGpDisplayDriver;
struct Point;
class PLPasStr;
struct Rect;
struct Window;

namespace PortabilityLayer
{
	struct WindowDef;
	struct Rect2i;

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
		virtual void DragWindow(Window *window, const Point &startPoint, const Rect &constraintRect) = 0;
		virtual void SetWindowTitle(Window *window, const PLPasStr &title) = 0;
		virtual Rect2i GetWindowFullRect(Window *window) const = 0;

		virtual void RenderFrame(IGpDisplayDriver *displayDriver) = 0;

		virtual void HandleScreenResolutionChange(uint32_t prevWidth, uint32_t prevHeight, uint32_t newWidth, uint32_t newHeight) = 0;

		static WindowManager *GetInstance();
	};
}
