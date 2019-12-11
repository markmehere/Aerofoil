#pragma once

#include "GpWindows.h"
#include "GpRingBuffer.h"

#include "IGpDisplayDriver.h"
#include "GpCoreDefs.h"
#include "GpDisplayDriverProperties.h"

#include "PixelFormat.h"

struct IDXGISwapChain1;

class GpDisplayDriverD3D11 : public IGpDisplayDriver
{
public:
	void Run() override;
	void Shutdown() override;

	void GetDisplayResolution(unsigned int *width, unsigned int *height, PortabilityLayer::PixelFormat *bpp) override;

	static GpDisplayDriverD3D11 *Create(const GpDisplayDriverProperties &properties);

private:
	GpDisplayDriverD3D11(const GpDisplayDriverProperties &properties);

	bool PresentFrameAndSync();

	IDXGISwapChain1 *m_swapChain;

	struct CompactedPresentHistoryItem
	{
		LARGE_INTEGER m_timestamp;
		unsigned int m_numFrames;
	};

	GpRingBuffer<CompactedPresentHistoryItem, 60> m_presentHistory;
	GpDisplayDriverProperties m_properties;

	LARGE_INTEGER m_syncTimeBase;
	LARGE_INTEGER m_QPFrequency;
	UINT m_expectedSyncDelta;
	bool m_isResettingSwapChain;

	LONGLONG m_frameTimeAccumulated;
	LONGLONG m_frameTimeSliceSize;

	DWORD m_windowWidth;
	DWORD m_windowHeight;

	GpFiber *m_vosFiber;
};
