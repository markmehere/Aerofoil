#include "GpDisplayDriverD3D11.h"
#include "GpWindows.h"
#include "GpFiber_Win32.h"

#include <d3d11.h>
#include <dxgi1_2.h>

#include <stdio.h>

#pragma comment (lib, "d3d11.lib")

void DebugPrintf(const char *fmt, ...)
{
	char buf[256];
	va_list argp;
	va_start(argp, fmt);
	vsnprintf_s(buf, 255, fmt, argp);
	OutputDebugString(buf);
	va_end(argp);
}

LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

void StartD3DForWindow(HWND hWnd, IDXGISwapChain1*& swapChain)
{
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc;

	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	swapChainDesc.BufferCount = 2;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC swapChainFullscreenDesc;

	ZeroMemory(&swapChainFullscreenDesc, sizeof(swapChainFullscreenDesc));

	swapChainFullscreenDesc.Windowed = TRUE;
	swapChainFullscreenDesc.RefreshRate.Numerator = 60;
	swapChainFullscreenDesc.RefreshRate.Denominator = 1;

	UINT flags = 0;
	const D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_9_1
	};

	flags |= D3D11_CREATE_DEVICE_DEBUG;

	ID3D11Device *device = NULL;
	ID3D11DeviceContext *context = NULL;

	D3D_FEATURE_LEVEL selectedFeatureLevel;

	HRESULT result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, featureLevels, sizeof(featureLevels) / sizeof(featureLevels[0]),
		D3D11_SDK_VERSION, &device, &selectedFeatureLevel, &context);

	IDXGIDevice2 *dxgiDevice = nullptr;
	result = device->QueryInterface(__uuidof(IDXGIDevice2), reinterpret_cast<void**>(&dxgiDevice));

	IDXGIAdapter *dxgiAdapter = nullptr;
	result = dxgiDevice->GetAdapter(&dxgiAdapter);

	IDXGIFactory2 *dxgiFactory = nullptr;
	result = dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory));

	result = dxgiFactory->CreateSwapChainForHwnd(device, hWnd, &swapChainDesc, nullptr, nullptr, &swapChain);
}

bool GpDisplayDriverD3D11::PresentFrameAndSync()
{
	DXGI_PRESENT_PARAMETERS presentParams;

	ZeroMemory(&presentParams, sizeof(presentParams));

	UINT lastPresentCount = 0;

	if (FAILED(m_swapChain->GetLastPresentCount(&lastPresentCount)))
		return false;

	if (FAILED(m_swapChain->Present1(1, 0, &presentParams)))
		return false;

	//DebugPrintf("r: %i\n", static_cast<int>(r));

	DXGI_FRAME_STATISTICS stats;
	if (FAILED(m_swapChain->GetFrameStatistics(&stats)))
		return false;

	if (stats.SyncQPCTime.QuadPart != 0)
	{
		if (m_syncTimeBase.QuadPart == 0)
			m_syncTimeBase = stats.SyncQPCTime;

		LARGE_INTEGER timestamp;
		timestamp.QuadPart = stats.SyncQPCTime.QuadPart - m_syncTimeBase.QuadPart;

		bool compacted = false;
		if (m_presentHistory.Size() > 0)
		{
			CompactedPresentHistoryItem &lastItem = m_presentHistory[m_presentHistory.Size() - 1];
			LONGLONG timeDelta = timestamp.QuadPart - lastItem.m_timestamp.QuadPart;

			if (timeDelta < 0)
				timeDelta = 0;	// This should never happen

			if (timeDelta * static_cast<LONGLONG>(m_properties.m_frameTimeLockDenominator) < m_QPFrequency.QuadPart * static_cast<LONGLONG>(m_properties.m_frameTimeLockNumerator))
			{
				lastItem.m_numFrames++;
				compacted = true;
			}
		}

		if (!compacted)
		{
			if (m_presentHistory.Size() == m_presentHistory.CAPACITY)
				m_presentHistory.RemoveFromStart();

			CompactedPresentHistoryItem *newItem = m_presentHistory.Append();
			newItem->m_timestamp = timestamp;
			newItem->m_numFrames = 1;
		}
	}

	if (m_presentHistory.Size() >= 2)
	{
		const size_t presentHistorySizeMinusOne = m_presentHistory.Size() - 1;
		unsigned int numFrames = 0;
		for (size_t i = 0; i < presentHistorySizeMinusOne; i++)
			numFrames += m_presentHistory[i].m_numFrames;

		LONGLONG timeFrame = m_presentHistory[presentHistorySizeMinusOne].m_timestamp.QuadPart - m_presentHistory[0].m_timestamp.QuadPart;

		unsigned int cancelledFrames = 0;
		LONGLONG cancelledTime = 0;

		const int overshootTolerance = 2;

		for (size_t i = 0; i < presentHistorySizeMinusOne; i++)
		{
			LONGLONG blockTimeframe = m_presentHistory[i + 1].m_timestamp.QuadPart - m_presentHistory[i].m_timestamp.QuadPart;
			unsigned int blockNumFrames = m_presentHistory[i].m_numFrames;

			if (blockTimeframe * static_cast<LONGLONG>(numFrames) >= timeFrame * static_cast<LONGLONG>(blockNumFrames) * overshootTolerance)
			{
				cancelledTime += blockTimeframe;
				cancelledFrames += blockNumFrames;
			}
		}

		numFrames -= cancelledFrames;
		timeFrame -= cancelledTime;

		// timeFrame / numFrames = Frame timestep
		// Unless Frame timestep is within the frame lock range, a.k.a.
		// timeFrame / numFrames / qpFreq >= minFrameTimeNum / minFrameTimeDenom

		bool isInFrameTimeLock = false;
		if (timeFrame * static_cast<LONGLONG>(m_properties.m_frameTimeLockMinDenominator) >= static_cast<LONGLONG>(numFrames) * static_cast<LONGLONG>(m_properties.m_frameTimeLockMinNumerator) * m_QPFrequency.QuadPart
			&& timeFrame * static_cast<LONGLONG>(m_properties.m_frameTimeLockMaxDenominator) <= static_cast<LONGLONG>(numFrames) * static_cast<LONGLONG>(m_properties.m_frameTimeLockMaxNumerator) * m_QPFrequency.QuadPart)
		{
			isInFrameTimeLock = true;
		}

		LONGLONG frameTimeStep = m_frameTimeSliceSize;
		if (!isInFrameTimeLock)
		{
			const int MAX_FRAMES_PER_STEP = 4;

			frameTimeStep = timeFrame / numFrames;
			if (frameTimeStep > m_frameTimeSliceSize * MAX_FRAMES_PER_STEP)
				frameTimeStep = m_frameTimeSliceSize * MAX_FRAMES_PER_STEP;
		}

		m_frameTimeAccumulated += frameTimeStep;
		while (m_frameTimeAccumulated >= m_frameTimeSliceSize)
		{
			m_properties.m_tickFunc(m_properties.m_tickFuncContext, m_vosFiber);
			m_frameTimeAccumulated -= m_frameTimeSliceSize;
		}
	}

	return true;
}

void GpDisplayDriverD3D11::Run()
{
	HWND hWnd;
	WNDCLASSEX wc;

	LPVOID fiber = ConvertThreadToFiberEx(this, 0);
	if (!fiber)
		return;	// ???

	m_vosFiber = new GpFiber_Win32(fiber);

	ZeroMemory(&wc, sizeof(wc));

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WinProc;
	wc.hInstance = g_gpWindowsGlobals.m_hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName = "GPD3D11WindowClass";

	RegisterClassEx(&wc);

	LONG windowStyle = WS_OVERLAPPEDWINDOW;
	HMENU menus = NULL;

	// TODO: Fix the resolution here
	RECT wr = { 0, 0, m_windowWidth, m_windowHeight };
	AdjustWindowRect(&wr, windowStyle, menus != NULL);

	hWnd = CreateWindowExW(NULL, L"GPD3D11WindowClass", L"GlidePort", WS_OVERLAPPEDWINDOW, 300, 300, wr.right - wr.left, wr.bottom - wr.top, NULL, menus, g_gpWindowsGlobals.m_hInstance, NULL);

	ShowWindow(hWnd, g_gpWindowsGlobals.m_nCmdShow);

	StartD3DForWindow(hWnd, m_swapChain);

	LARGE_INTEGER lastTimestamp;
	memset(&lastTimestamp, 0, sizeof(lastTimestamp));

	MSG msg;
	for (;;)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);

			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
				break;
		}
		else
		{
			PresentFrameAndSync();
		}
	}

	// Exit
	ConvertFiberToThread();
}

void GpDisplayDriverD3D11::Shutdown()
{
	delete this;
}

void GpDisplayDriverD3D11::GetDisplayResolution(unsigned int *width, unsigned int *height, PortabilityLayer::PixelFormat *pixelFormat)
{
	if (width)
		*width = m_windowWidth;
	if (height)
		*height = m_windowHeight;
	if (pixelFormat)
		*pixelFormat = PortabilityLayer::PixelFormat_8BitStandard;
}

GpDisplayDriverD3D11 *GpDisplayDriverD3D11::Create(const GpDisplayDriverProperties &properties)
{
	return new GpDisplayDriverD3D11(properties);
}

GpDisplayDriverD3D11::GpDisplayDriverD3D11(const GpDisplayDriverProperties &properties)
	: m_properties(properties)
	, m_frameTimeAccumulated(0)
	, m_windowWidth(640)
	, m_windowHeight(480)
	, m_vosFiber(nullptr)
{
	memset(&m_syncTimeBase, 0, sizeof(m_syncTimeBase));

	QueryPerformanceFrequency(&m_QPFrequency);

	m_frameTimeSliceSize = m_QPFrequency.QuadPart * static_cast<LONGLONG>(properties.m_frameTimeLockNumerator) / static_cast<LONGLONG>(properties.m_frameTimeLockDenominator);
}
