#pragma once

#include "GpWindows.h"
#include "GpRingBuffer.h"

#include "IGpDisplayDriver.h"
#include "GpCoreDefs.h"
#include "GpDisplayDriverProperties.h"

struct IDXGISwapChain1;

class GpDisplayDriverD3D11 : public IGpDisplayDriver
{
public:
	void Run() override;
	void Shutdown() override;

	void GetDisplayResolution(unsigned int &width, unsigned int &height) override;

	static GpDisplayDriverD3D11 *Create(const GpDisplayDriverProperties &properties);

private:
	GpDisplayDriverD3D11(const GpDisplayDriverProperties &properties);

	bool PresentFrameAndSync();

	IDXGISwapChain1 *m_SwapChain;

	struct CompactedPresentHistoryItem
	{
		LARGE_INTEGER m_Timestamp;
		unsigned int m_NumFrames;
	};

	GpRingBuffer<CompactedPresentHistoryItem, 60> m_PresentHistory;
	GpDisplayDriverProperties m_Properties;

	LARGE_INTEGER m_SyncTimeBase;
	LARGE_INTEGER m_QPFrequency;
	UINT m_ExpectedSyncDelta;
	bool m_IsResettingSwapChain;

	LONGLONG m_FrameTimeAccumulated;
	LONGLONG m_FrameTimeSliceSize;

	DWORD m_windowWidth;
	DWORD m_windowHeight;

	GpFiber *m_vosFiber;
};
