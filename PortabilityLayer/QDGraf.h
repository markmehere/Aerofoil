#pragma once

#include <stdint.h>

#include "GpPixelFormat.h"
#include "PLHandle.h"
#include "QDPort.h"
#include "RGBAColor.h"

namespace PortabilityLayer
{
	struct AntiAliasTable;
	class FontFamily;
	struct RGBAColor;
	class RenderedFont;
	class ResolveCachingColor;
	class ScanlineMask;
	class FontSpec;
}

struct PixMap;
struct BitmapImage;
struct Point;
struct Rect;
struct IGpDisplayDriver;
struct IGpDisplayDriverSurface;
class PLPasStr;

struct DrawSurface
{
	DrawSurface()
		: m_port(PortabilityLayer::QDPortType_DrawSurface)
		, m_ddSurface(nullptr)
		, m_cachedAAColor(PortabilityLayer::RGBAColor::Create(0, 0, 0, 255))
		, m_cachedAATable(nullptr)
	{
	}

	explicit DrawSurface(PortabilityLayer::QDPortType overridePortType)
		: m_port(overridePortType)
		, m_ddSurface(nullptr)
		, m_cachedAAColor(PortabilityLayer::RGBAColor::Create(0, 0, 0, 255))
		, m_cachedAATable(nullptr)
	{
	}

	~DrawSurface();

	PLError_t Init(const Rect &rect, GpPixelFormat_t pixelFormat)
	{
		if (PLError_t errorCode = m_port.Init(rect, pixelFormat))
			return errorCode;

		return PLErrors::kNone;
	}

	bool Resize(const Rect &rect)
	{
		return m_port.Resize(rect);
	}

	void PushToDDSurface(IGpDisplayDriver *displayDriver);

	void FillRect(const Rect &rect, PortabilityLayer::ResolveCachingColor &cacheColor);
	void FillRectWithMaskPattern8x8(const Rect &rect, const uint8_t *pattern, PortabilityLayer::ResolveCachingColor &cacheColor);
	void FrameRect(const Rect &rect, PortabilityLayer::ResolveCachingColor &cacheColor);
	void FrameRoundRect(const Rect &rect, int quadrantWidth, int quadrantHeight, PortabilityLayer::ResolveCachingColor &cacheColor);
	void InvertFrameRect(const Rect &rect, const uint8_t *pattern);
	void InvertFillRect(const Rect &rect, const uint8_t *pattern);

	void FillEllipse(const Rect &rect, PortabilityLayer::ResolveCachingColor &cacheColor);
	void FillEllipseWithMaskPattern(const Rect &rect, const uint8_t *pattern, PortabilityLayer::ResolveCachingColor &cacheColor);
	void FrameEllipse(const Rect &rect, PortabilityLayer::ResolveCachingColor &cacheColor);

	void FillScanlineMask(const PortabilityLayer::ScanlineMask *scanlineMask, PortabilityLayer::ResolveCachingColor &cacheColor);
	void FillScanlineMaskWithMaskPattern(const PortabilityLayer::ScanlineMask *scanlineMask, const uint8_t *pattern, PortabilityLayer::ResolveCachingColor &cacheColor);

	void DrawLine(const Point &a, const Point &b, PortabilityLayer::ResolveCachingColor &cacheColor);

	void DrawString(const Point &point, const PLPasStr &str, PortabilityLayer::ResolveCachingColor &cacheColor, PortabilityLayer::RenderedFont *font);
	void DrawStringConstrained(const Point &point, const PLPasStr &str, const Rect &constraintRect, PortabilityLayer::ResolveCachingColor &cacheColor, PortabilityLayer::RenderedFont *font);
	void DrawStringWrap(const Point &point, const Rect &constrainRect, const PLPasStr &str, PortabilityLayer::ResolveCachingColor &cacheColor, PortabilityLayer::RenderedFont *font);

	void DrawPicture(THandle<BitmapImage> pictHandle, const Rect &rect);

	IGpDisplayDriverSurface *m_ddSurface;

	PortabilityLayer::AntiAliasTable *m_cachedAATable;
	PortabilityLayer::RGBAColor m_cachedAAColor;

	PortabilityLayer::QDPort m_port;
};
