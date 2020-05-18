#pragma once

#include "GpWindows.h"
#include "GpRingBuffer.h"

#include "IGpDisplayDriver.h"
#include "GpCoreDefs.h"
#include "GpDisplayDriverProperties.h"
#include "GpComPtr.h"

#include "GpPixelFormat.h"

struct GpWindowsGlobals;
struct IGpCursor_Win32;
struct IGpCursor;
struct IGpFiber;

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

	void GetDisplayResolution(unsigned int *width, unsigned int *height, GpPixelFormat_t *bpp) override;

	IGpDisplayDriverSurface *CreateSurface(size_t width, size_t height, GpPixelFormat_t pixelFormat) override;
	void DrawSurface(IGpDisplayDriverSurface *surface, int32_t x, int32_t y, size_t width, size_t height, const GpDisplayDriverSurfaceEffects *effects) override;

	IGpCursor *LoadCursor(bool isColor, int cursorID) override;
	void SetCursor(IGpCursor *cursor) override;
	void SetStandardCursor(EGpStandardCursor_t standardCursor) override;

	void UpdatePalette(const void *paletteData) override;

	void SetBackgroundColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) override;

	void RequestToggleFullScreen(uint32_t timestamp) override;

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

	struct DrawQuadPixelConstants
	{
		float m_modulation[4];
	};

	struct ScaleQuadPixelConstants
	{
		float m_dx;
		float m_dy;
		float m_unused[2];
	};

	struct CompactedPresentHistoryItem
	{
		LARGE_INTEGER m_timestamp;
		unsigned int m_numFrames;
	};

	GpDisplayDriverD3D11(const GpDisplayDriverProperties &properties);
	~GpDisplayDriverD3D11();

	bool DetachSwapChain();
	bool InitBackBuffer(uint32_t virtualWidth, uint32_t virtualHeight);
	bool InitResources(uint32_t virtualWidth, uint32_t virtualHeight);
	GpDisplayDriverTickStatus_t PresentFrameAndSync();
	void ScaleVirtualScreen();

	void SynchronizeCursors();
	void ChangeToCursor(HCURSOR cursor);
	void ChangeToStandardCursor(EGpStandardCursor_t cursor);

	void BecomeFullScreen(LONG &windowStyle);
	void BecomeWindowed(LONG &windowStyle);

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
	GpComPtr<ID3D11PixelShader> m_scaleQuadPixelShader;
	GpComPtr<ID3D11Buffer> m_drawQuadVertexConstantBuffer;
	GpComPtr<ID3D11Buffer> m_drawQuadPixelConstantBuffer;
	GpComPtr<ID3D11Buffer> m_scaleQuadPixelConstantBuffer;
	GpComPtr<ID3D11DepthStencilState> m_drawQuadDepthStencilState;
	GpComPtr<ID3D11SamplerState> m_nearestNeighborSamplerState;
	GpComPtr<ID3D11Texture1D> m_paletteTexture;
	GpComPtr<ID3D11ShaderResourceView> m_paletteTextureSRV;

	GpComPtr<ID3D11Texture2D> m_backBufferTexture;
	GpComPtr<ID3D11RenderTargetView> m_backBufferRTV;

	GpComPtr<ID3D11Texture2D> m_virtualScreenTexture;
	GpComPtr<ID3D11RenderTargetView> m_virtualScreenTextureRTV;
	GpComPtr<ID3D11ShaderResourceView> m_virtualScreenTextureSRV;

	GpRingBuffer<CompactedPresentHistoryItem, 60> m_presentHistory;
	GpDisplayDriverProperties m_properties;

	LARGE_INTEGER m_syncTimeBase;
	LARGE_INTEGER m_QPFrequency;
	UINT m_expectedSyncDelta;
	bool m_isResettingSwapChain;

	bool m_isFullScreen;
	bool m_isFullScreenDesired;
	RECT m_windowModeRevertRect;
	uint32_t m_lastFullScreenToggleTimeStamp;

	LONGLONG m_frameTimeAccumulated;
	LONGLONG m_frameTimeSliceSize;

	DWORD m_windowWidthPhysical;	// Physical resolution is the resolution of the actual window
	DWORD m_windowHeightPhysical;
	DWORD m_windowWidthVirtual;		// Virtual resolution is the resolution reported to teh game
	DWORD m_windowHeightVirtual;
	float m_pixelScaleX;
	float m_pixelScaleY;

	IGpCursor_Win32 *m_activeCursor;
	IGpCursor_Win32 *m_pendingCursor;
	EGpStandardCursor_t m_currentStandardCursor;
	EGpStandardCursor_t m_pendingStandardCursor;
	bool m_mouseIsInClientArea;

	IGpFiber *m_vosFiber;
	GpWindowsGlobals *m_osGlobals;

	HCURSOR m_arrowCursor;
	HCURSOR m_waitCursor;
	HCURSOR m_ibeamCursor;

	float m_bgColor[4];
};
