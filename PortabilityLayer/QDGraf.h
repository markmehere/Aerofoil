#pragma once

#include <stdint.h>

#include "GpPixelFormat.h"
#include "PLHandle.h"
#include "QDState.h"
#include "QDPort.h"

namespace PortabilityLayer
{
	struct AntiAliasTable;
	class FontFamily;
	struct RGBAColor;
	class ScanlineMask;
}

struct PixMap;
struct BitmapImage;
struct Point;
struct Rect;
struct IGpDisplayDriver;
struct IGpDisplayDriverSurface;
class PLPasStr;

struct DrawSurface final
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

	void FillRect(const Rect &rect);
	void FillRectWithPattern8x8(const Rect &rect, const uint8_t *pattern);
	void FrameRect(const Rect &rect);
	void FrameRoundRect(const Rect &rect, int quadrantWidth, int quadrantHeight);
	void InvertFrameRect(const Rect &rect, const uint8_t *pattern);
	void InvertFillRect(const Rect &rect, const uint8_t *pattern);

	void FillEllipse(const Rect &rect);
	void FrameEllipse(const Rect &rect);

	void FillScanlineMask(const PortabilityLayer::ScanlineMask *scanlineMask);

	void DrawLine(const Point &a, const Point &b);

	void SetForeColor(const PortabilityLayer::RGBAColor &color);
	const PortabilityLayer::RGBAColor &GetForeColor() const;

	void SetBackColor(const PortabilityLayer::RGBAColor &color);
	const PortabilityLayer::RGBAColor &GetBackColor() const;

	void SetApplicationFont(int size, int variationFlags);
	void SetSystemFont(int size, int variationFlags);
	void DrawString(const Point &point, const PLPasStr &str, bool aa);
	void DrawStringConstrained(const Point &point, const PLPasStr &str, bool aa, const Rect &constraintRect);
	void DrawStringWrap(const Point &point, const Rect &constrainRect, const PLPasStr &str, bool aa);

	size_t MeasureString(const PLPasStr &str);
	int32_t MeasureFontAscender();
	int32_t MeasureFontLineGap();

	void DrawPicture(THandle<BitmapImage> pictHandle, const Rect &rect);

	void SetPattern8x8(const uint8_t *pattern);
	void ClearPattern();

	void SetMaskMode(bool maskMode);

	Rect GetClipRect() const;
	void SetClipRect(const Rect &rect);

	// Must be the first item
	PortabilityLayer::QDPort m_port;

	IGpDisplayDriverSurface *m_ddSurface;

	PortabilityLayer::AntiAliasTable *m_cachedAATable;
	PortabilityLayer::RGBAColor m_cachedAAColor;
};
