#pragma once

#include "GpPixelFormat.h"
#include "EGpStandardCursor.h"

struct IGpDisplayDriverSurface;
struct IGpCursor;

// Display drivers are responsible for timing and calling the game tick function.
struct IGpDisplayDriver
{
public:
	virtual void Run() = 0;
	virtual void Shutdown() = 0;

	virtual void GetDisplayResolution(unsigned int *width, unsigned int *height, GpPixelFormat_t *bpp) = 0;

	virtual IGpDisplayDriverSurface *CreateSurface(size_t width, size_t height, GpPixelFormat_t pixelFormat) = 0;
	virtual void DrawSurface(IGpDisplayDriverSurface *surface, int32_t x, int32_t y, size_t width, size_t height) = 0;

	virtual IGpCursor *LoadCursor(bool isColor, int cursorID) = 0;
	virtual void SetCursor(IGpCursor *cursor) = 0;
	virtual void SetStandardCursor(EGpStandardCursor_t standardCursor) = 0;

	virtual void UpdatePalette(const void *paletteData) = 0;
};
