#pragma once

#include "GpPixelFormat.h"
#include "EGpStandardCursor.h"

struct IGpDisplayDriverSurface;
struct IGpCursor;

struct GpDisplayDriverSurfaceEffects
{
	GpDisplayDriverSurfaceEffects();

	bool m_darken;
	bool m_flicker;
	int32_t m_flickerAxisX;
	int32_t m_flickerAxisY;
	int32_t m_flickerStartThreshold;
	int32_t m_flickerEndThreshold;
	float m_desaturation;
};

// Display drivers are responsible for timing and calling the game tick function.
struct IGpDisplayDriver
{
public:
	virtual void Run() = 0;
	virtual void Shutdown() = 0;

	virtual void GetDisplayResolution(unsigned int *width, unsigned int *height, GpPixelFormat_t *bpp) = 0;

	virtual IGpDisplayDriverSurface *CreateSurface(size_t width, size_t height, GpPixelFormat_t pixelFormat) = 0;
	virtual void DrawSurface(IGpDisplayDriverSurface *surface, int32_t x, int32_t y, size_t width, size_t height, const GpDisplayDriverSurfaceEffects *effects) = 0;

	virtual IGpCursor *LoadCursor(bool isColor, int cursorID) = 0;
	virtual void SetCursor(IGpCursor *cursor) = 0;
	virtual void SetStandardCursor(EGpStandardCursor_t standardCursor) = 0;

	virtual void UpdatePalette(const void *paletteData) = 0;

	virtual void SetBackgroundColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) = 0;

	virtual void RequestToggleFullScreen(uint32_t timestamp) = 0;
};

inline GpDisplayDriverSurfaceEffects::GpDisplayDriverSurfaceEffects()
	: m_darken(false)
	, m_flicker(false)
	, m_flickerAxisX(0)
	, m_flickerAxisY(0)
	, m_flickerStartThreshold(0)
	, m_flickerEndThreshold(0)
	, m_desaturation(0)
{
}
