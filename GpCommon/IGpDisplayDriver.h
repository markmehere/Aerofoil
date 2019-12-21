#pragma once

#include "PixelFormat.h"

struct IGpDisplayDriverSurface;

// Display drivers are responsible for timing and calling the game tick function.
struct IGpDisplayDriver
{
public:
	virtual void Run() = 0;
	virtual void Shutdown() = 0;

	virtual void GetDisplayResolution(unsigned int *width, unsigned int *height, PortabilityLayer::PixelFormat *bpp) = 0;

	virtual IGpDisplayDriverSurface *CreateSurface(size_t width, size_t height, PortabilityLayer::PixelFormat pixelFormat) = 0;
	virtual void DrawSurface(IGpDisplayDriverSurface *surface, size_t x, size_t y, size_t width, size_t height) = 0;

	virtual void UpdatePalette(const void *paletteData) = 0;
};
