#pragma once

#include "GpWindows.h"
#include "GpRingBuffer.h"

#include "IGpDisplayDriver.h"
#include "GpCoreDefs.h"
#include "GpDisplayDriverProperties.h"
#include "GpComPtr.h"

#include "PixelFormat.h"

struct GpWindowsGlobals;
class GpColorCursor_Win32;

struct IDXGISwapChain1;
struct ID3D11Buffer;
struct ID3D11DepthStencilState;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11InputLayout;
struct ID3D11PixelShader;
struct ID3D11RenderTargetView;
struct ID3D11SamplerState;
struct ID3D11ShaderResourceView;
struct ID3D11Texture1D;
struct ID3D11Texture2D;
struct ID3D11VertexShader;

class GpDisplayDriverD3D11 : public IGpDisplayDriver
{
public:
	void Run() override;
	void Shutdown() override;

	void GetDisplayResolution(unsigned int *width, unsigned int *height, PortabilityLayer::PixelFormat *bpp) override;

	IGpDisplayDriverSurface *CreateSurface(size_t width, size_t height, PortabilityLayer::PixelFormat pixelFormat) override;
	void DrawSurface(IGpDisplayDriverSurface *surface, size_t x, size_t y, size_t width, size_t height) override;

	IGpColorCursor *LoadColorCursor(int cursorID) override;
	void SetColorCursor(IGpColorCursor *colorCursor) override;
	void SetStandardCursor(EGpStandardCursor_t standardCursor) override;

	void UpdatePalette(const void *paletteData) override;

	static GpDisplayDriverD3D11 *Create(const GpDisplayDriverProperties &properties);

private:
	struct DrawQuadVertexConstants
	{
		float m_ndcOriginX;
		float m_ndcOriginY;
		float m_ndcWidth;
		float m_ndcHeight;

		float m_surfaceDimensionX;
		float m_surfaceDimensionY;
		float m_unused[2];
	};

	struct CompactedPresentHistoryItem
	{
		LARGE_INTEGER m_timestamp;
		unsigned int m_numFrames;
	};

	GpDisplayDriverD3D11(const GpDisplayDriverProperties &properties);
	~GpDisplayDriverD3D11();

	bool InitResources();
	bool PresentFrameAndSync();

	void SynchronizeCursors();
	void ChangeToCursor(HCURSOR cursor);
	void ChangeToStandardCursor(EGpStandardCursor_t cursor);

	GpComPtr<IDXGISwapChain1> m_swapChain;
	GpComPtr<ID3D11Device> m_device;
	GpComPtr<ID3D11DeviceContext> m_deviceContext;
	GpComPtr<ID3D11Buffer> m_quadIndexBuffer;
	GpComPtr<ID3D11Buffer> m_quadVertexBuffer;
	GpComPtr<ID3D11InputLayout> m_drawQuadInputLayout;
	GpComPtr<ID3D11VertexShader> m_drawQuadVertexShader;
	GpComPtr<ID3D11PixelShader> m_drawQuadPalettePixelShader;
	GpComPtr<ID3D11PixelShader> m_drawQuad15BitPixelShader;
	GpComPtr<ID3D11PixelShader> m_drawQuadRGBPixelShader;
	GpComPtr<ID3D11Buffer> m_drawQuadVertexConstantBuffer;
	GpComPtr<ID3D11DepthStencilState> m_drawQuadDepthStencilState;
	GpComPtr<ID3D11SamplerState> m_nearestNeighborSamplerState;
	GpComPtr<ID3D11Texture1D> m_paletteTexture;
	GpComPtr<ID3D11ShaderResourceView> m_paletteTextureSRV;

	GpComPtr<ID3D11Texture2D> m_backBufferTexture;
	GpComPtr<ID3D11RenderTargetView> m_backBufferRTV;

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

	GpColorCursor_Win32 *m_activeCursor;
	GpColorCursor_Win32 *m_pendingCursor;
	EGpStandardCursor_t m_currentStandardCursor;
	EGpStandardCursor_t m_pendingStandardCursor;
	bool m_mouseIsInClientArea;

	GpFiber *m_vosFiber;
	GpWindowsGlobals *m_osGlobals;

	HCURSOR m_arrowCursor;
	HWND m_hwnd;
};
