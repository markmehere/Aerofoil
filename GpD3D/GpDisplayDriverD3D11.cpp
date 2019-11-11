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

	if (FAILED(m_SwapChain->GetLastPresentCount(&lastPresentCount)))
		return false;

	if (FAILED(m_SwapChain->Present1(1, 0, &presentParams)))
		return false;

	//DebugPrintf("r: %i\n", static_cast<int>(r));

	DXGI_FRAME_STATISTICS stats;
	if (FAILED(m_SwapChain->GetFrameStatistics(&stats)))
		return false;

	if (stats.SyncQPCTime.QuadPart != 0)
	{
		if (m_SyncTimeBase.QuadPart == 0)
			m_SyncTimeBase = stats.SyncQPCTime;

		LARGE_INTEGER timestamp;
		timestamp.QuadPart = stats.SyncQPCTime.QuadPart - m_SyncTimeBase.QuadPart;

		bool compacted = false;
		if (m_PresentHistory.Size() > 0)
		{
			CompactedPresentHistoryItem &lastItem = m_PresentHistory[m_PresentHistory.Size() - 1];
			LONGLONG timeDelta = timestamp.QuadPart - lastItem.m_Timestamp.QuadPart;

			if (timeDelta < 0)
				timeDelta = 0;	// This should never happen

			if (timeDelta * static_cast<LONGLONG>(m_Properties.m_FrameTimeLockDenominator) < m_QPFrequency.QuadPart * static_cast<LONGLONG>(m_Properties.m_FrameTimeLockNumerator))
			{
				lastItem.m_NumFrames++;
				compacted = true;
			}
		}

		if (!compacted)
		{
			if (m_PresentHistory.Size() == m_PresentHistory.CAPACITY)
				m_PresentHistory.RemoveFromStart();

			CompactedPresentHistoryItem *newItem = m_PresentHistory.Append();
			newItem->m_Timestamp = timestamp;
			newItem->m_NumFrames = 1;
		}
	}

	if (m_PresentHistory.Size() >= 2)
	{
		const size_t presentHistorySizeMinusOne = m_PresentHistory.Size() - 1;
		unsigned int numFrames = 0;
		for (size_t i = 0; i < presentHistorySizeMinusOne; i++)
			numFrames += m_PresentHistory[i].m_NumFrames;

		LONGLONG timeFrame = m_PresentHistory[presentHistorySizeMinusOne].m_Timestamp.QuadPart - m_PresentHistory[0].m_Timestamp.QuadPart;

		unsigned int cancelledFrames = 0;
		LONGLONG cancelledTime = 0;

		const int overshootTolerance = 2;

		for (size_t i = 0; i < presentHistorySizeMinusOne; i++)
		{
			LONGLONG blockTimeframe = m_PresentHistory[i + 1].m_Timestamp.QuadPart - m_PresentHistory[i].m_Timestamp.QuadPart;
			unsigned int blockNumFrames = m_PresentHistory[i].m_NumFrames;

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
		if (timeFrame * static_cast<LONGLONG>(m_Properties.m_FrameTimeLockMinDenominator) >= static_cast<LONGLONG>(numFrames) * static_cast<LONGLONG>(m_Properties.m_FrameTimeLockMinNumerator) * m_QPFrequency.QuadPart
			&& timeFrame * static_cast<LONGLONG>(m_Properties.m_FrameTimeLockMaxDenominator) <= static_cast<LONGLONG>(numFrames) * static_cast<LONGLONG>(m_Properties.m_FrameTimeLockMaxNumerator) * m_QPFrequency.QuadPart)
		{
			isInFrameTimeLock = true;
		}

		LONGLONG frameTimeStep = m_FrameTimeSliceSize;
		if (!isInFrameTimeLock)
		{
			const int MAX_FRAMES_PER_STEP = 4;

			frameTimeStep = timeFrame / numFrames;
			if (frameTimeStep > m_FrameTimeSliceSize * MAX_FRAMES_PER_STEP)
				frameTimeStep = m_FrameTimeSliceSize * MAX_FRAMES_PER_STEP;
		}

		m_FrameTimeAccumulated += frameTimeStep;
		while (m_FrameTimeAccumulated >= m_FrameTimeSliceSize)
		{
			m_Properties.m_TickFunc(m_Properties.m_TickFuncContext, m_vosFiber);
			m_FrameTimeAccumulated -= m_FrameTimeSliceSize;
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

	StartD3DForWindow(hWnd, m_SwapChain);

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

void GpDisplayDriverD3D11::GetDisplayResolution(unsigned int &width, unsigned int &height)
{
	width = m_windowWidth;
	height = m_windowHeight;
}

GpDisplayDriverD3D11 *GpDisplayDriverD3D11::Create(const GpDisplayDriverProperties &properties)
{
	return new GpDisplayDriverD3D11(properties);
}

GpDisplayDriverD3D11::GpDisplayDriverD3D11(const GpDisplayDriverProperties &properties)
	: m_Properties(properties)
	, m_FrameTimeAccumulated(0)
	, m_windowWidth(640)
	, m_windowHeight(480)
	, m_vosFiber(nullptr)
{
	memset(&m_SyncTimeBase, 0, sizeof(m_SyncTimeBase));

	QueryPerformanceFrequency(&m_QPFrequency);

	m_FrameTimeSliceSize = m_QPFrequency.QuadPart * static_cast<LONGLONG>(properties.m_FrameTimeLockNumerator) / static_cast<LONGLONG>(properties.m_FrameTimeLockDenominator);
}
