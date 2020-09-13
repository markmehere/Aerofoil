#include "GpApplicationName.h"
#include "GpDisplayDriverD3D11.h"
#include "GpDisplayDriverSurfaceD3D11.h"
#include "GpVOSEvent.h"
#include "GpWindows.h"
#include "IGpCursor_Win32.h"
#include "IGpFiber.h"
#include "IGpVOSEventQueue.h"

#include "IGpLogDriver.h"

#include <d3d11.h>
#include <dxgi1_2.h>
#include <float.h>
#include <emmintrin.h>

#include <stdio.h>
#include <new>

#pragma comment (lib, "d3d11.lib")

static GpDisplayDriverSurfaceEffects gs_defaultEffects;

static const char *kPrefsIdentifier = "GpDisplayDriverD3D11";
static uint32_t kPrefsVersion = 1;

struct GpDisplayDriverD3D11_Prefs
{
	bool m_isFullScreen;
};

namespace GpBinarizedShaders
{
	extern const unsigned char *g_drawQuadV_D3D11[2];

	extern const unsigned char *g_drawQuadPaletteP_D3D11[2];
	extern const unsigned char *g_drawQuadRGBP_D3D11[2];
	extern const unsigned char *g_drawQuad15BitP_D3D11[2];
	
	extern const unsigned char *g_drawQuadPaletteICCP_D3D11[2];
	extern const unsigned char *g_drawQuadRGBICCP_D3D11[2];
	extern const unsigned char *g_drawQuad15BitICCP_D3D11[2];

	extern const unsigned char *g_scaleQuadP_D3D11[2];
}

struct GpShaderCodeBlob
{
	const void *m_data;
	size_t m_size;
};

static GpShaderCodeBlob GetBinarizedShader(const unsigned char **shaderPointers)
{
	GpShaderCodeBlob blob;
	blob.m_data = shaderPointers[0];
	blob.m_size = shaderPointers[1] - shaderPointers[0];

	return blob;
}

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
	case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
		}
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

bool InitSwapChainForWindow(HWND hWnd, ID3D11Device *device, GpComPtr<IDXGISwapChain1>& outSwapChain)
{
	outSwapChain = nullptr;

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc;

	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	swapChainDesc.BufferCount = 2;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

	HRESULT result;

	IDXGIDevice2 *dxgiDevice = nullptr;
	result = device->QueryInterface(__uuidof(IDXGIDevice2), reinterpret_cast<void**>(&dxgiDevice));
	if (result != S_OK)
		return false;

	IDXGIAdapter *dxgiAdapter = nullptr;
	result = dxgiDevice->GetAdapter(&dxgiAdapter);
	if (result != S_OK)
		return false;

	IDXGIFactory2 *dxgiFactory = nullptr;
	result = dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory));
	if (result != S_OK)
		return false;

	IDXGISwapChain1 *swapChain = nullptr;
	result = dxgiFactory->CreateSwapChainForHwnd(device, hWnd, &swapChainDesc, nullptr, nullptr, &swapChain);
	if (result != S_OK)
		return false;

	result = dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);
	if (result != S_OK)
		return false;

	outSwapChain = swapChain;

	return true;
}

bool ResizeSwapChain(IDXGISwapChain1 *swapChain, UINT width, UINT height)
{
	HRESULT result;

	result = swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
	if (result != S_OK)
		return false;

	return true;
}

void StartD3DForWindow(HWND hWnd, GpComPtr<IDXGISwapChain1>& outSwapChain, GpComPtr<ID3D11Device>& outDevice, GpComPtr<ID3D11DeviceContext>& outContext, IGpLogDriver *logger)
{
	DXGI_SWAP_CHAIN_FULLSCREEN_DESC swapChainFullscreenDesc;

	ZeroMemory(&swapChainFullscreenDesc, sizeof(swapChainFullscreenDesc));

	swapChainFullscreenDesc.Windowed = TRUE;
	swapChainFullscreenDesc.RefreshRate.Numerator = 60;
	swapChainFullscreenDesc.RefreshRate.Denominator = 1;

	UINT flags = 0;
	const D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_10_0
	};

#if GP_DEBUG_CONFIG
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	ID3D11Device *device = NULL;
	ID3D11DeviceContext *context = NULL;

	D3D_FEATURE_LEVEL selectedFeatureLevel;

	HRESULT result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, featureLevels, sizeof(featureLevels) / sizeof(featureLevels[0]),
		D3D11_SDK_VERSION, &device, &selectedFeatureLevel, &context);

	if (logger)
	{
		if (result == S_OK)
			logger->Printf(IGpLogDriver::Category_Information, "StartD3DForWindow: D3D11CreateDevice succeeded.  Selected feature level is %i", static_cast<int>(selectedFeatureLevel));
		else
			logger->Printf(IGpLogDriver::Category_Error, "StartD3DForWindow: D3D11CreateDevice failed with code %lx", result);
	}

	InitSwapChainForWindow(hWnd, device, outSwapChain);

	// GP TODO: Fix the error handling here, it's bad...
	outDevice = device;
	outContext = context;
}

bool ResizeD3DWindow(HWND hWnd, DWORD &windowWidth, DWORD &windowHeight, LONG desiredWidth, LONG desiredHeight, DWORD windowStyle, HMENU menus, IGpLogDriver *logger)
{
	if (logger)
		logger->Printf(IGpLogDriver::Category_Information, "ResizeD3DWindow: %i x %i", static_cast<int>(desiredWidth), static_cast<int>(desiredHeight));

	if (desiredWidth < 640)
		desiredWidth = 640;
	else if (desiredWidth > MAXDWORD)
		desiredWidth = MAXDWORD;

	if (desiredHeight < 480)
		desiredHeight = 480;
	else if (desiredHeight > MAXDWORD)
		desiredHeight = MAXDWORD;

	if (logger)
		logger->Printf(IGpLogDriver::Category_Information, "ResizeD3DWindow: Adjusted dimensions: %i x %i", static_cast<int>(desiredWidth), static_cast<int>(desiredHeight));

	RECT windowRect;
	GetClientRect(hWnd, &windowRect);
	windowRect.right = windowRect.left + desiredWidth;
	windowRect.bottom = windowRect.top + desiredHeight;

	LONG_PTR style = GetWindowLongPtrA(hWnd, GWL_STYLE);

	if (!AdjustWindowRect(&windowRect, static_cast<DWORD>(style), menus != nullptr))
	{
		if (logger)
			logger->Printf(IGpLogDriver::Category_Error, "ResizeD3DWindow: AdjustWindowRect failed");

		return false;
	}

	if (!SetWindowPos(hWnd, HWND_TOP, windowRect.left, windowRect.top, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, SWP_NOZORDER | SWP_NOMOVE))
	{
		if (logger)
			logger->Printf(IGpLogDriver::Category_Error, "ResizeD3DWindow: SetWindowPos failed");

		return false;
	}

	windowWidth = desiredWidth;
	windowHeight = desiredHeight;

	return true;
}

bool GpDisplayDriverD3D11::DetachSwapChain()
{
	IGpLogDriver *logger = m_properties.m_logger;

	if (logger)
		logger->Printf(IGpLogDriver::Category_Information, "GpDisplayDriverD3D11::DetachSwapChain");

	m_deviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	m_backBufferRTV = nullptr;
	m_backBufferTexture = nullptr;

	m_deviceContext->ClearState();
	m_deviceContext->Flush();

	return true;
}

bool GpDisplayDriverD3D11::InitBackBuffer(uint32_t virtualWidth, uint32_t virtualHeight)
{
	IGpLogDriver *logger = m_properties.m_logger;

	if (logger)
		logger->Printf(IGpLogDriver::Category_Information, "GpDisplayDriverD3D11::InitBackBuffer");

	m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(m_backBufferTexture.GetMutablePtr()));

	{
		D3D11_TEXTURE2D_DESC bbTextureDesc;
		m_backBufferTexture->GetDesc(&bbTextureDesc);

		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;

		m_backBufferRTV = nullptr;
		HRESULT result = m_device->CreateRenderTargetView(m_backBufferTexture, &rtvDesc, m_backBufferRTV.GetMutablePtr());
		if (result != S_OK)
		{
			if (logger)
				logger->Printf(IGpLogDriver::Category_Error, "GpDisplayDriverD3D11::InitBackBuffer: CreateRenderTargetView for back buffer failed with code %lx", result);

			return false;
		}
	}

	DXGI_FORMAT vbbFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	{
		D3D11_TEXTURE2D_DESC vbbTextureDesc;
		vbbTextureDesc.Width = static_cast<UINT>(virtualWidth);
		vbbTextureDesc.Height = static_cast<UINT>(virtualHeight);
		vbbTextureDesc.MipLevels = 1;
		vbbTextureDesc.ArraySize = 1;
		vbbTextureDesc.Format = vbbFormat;
		vbbTextureDesc.SampleDesc.Count = 1;
		vbbTextureDesc.SampleDesc.Quality = 0;
		vbbTextureDesc.Usage = D3D11_USAGE_DEFAULT;
		vbbTextureDesc.BindFlags = (D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET);
		vbbTextureDesc.CPUAccessFlags = 0;
		vbbTextureDesc.MiscFlags = 0;

		m_virtualScreenTexture = nullptr;
		HRESULT result = m_device->CreateTexture2D(&vbbTextureDesc, nullptr, m_virtualScreenTexture.GetMutablePtr());
		if (result != S_OK)
		{
			if (logger)
				logger->Printf(IGpLogDriver::Category_Error, "GpDisplayDriverD3D11::InitBackBuffer: CreateTexture2D for virtual screen texture failed with code %lx", result);

			return false;
		}
	}

	{
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
		rtvDesc.Format = vbbFormat;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;

		m_virtualScreenTextureRTV = nullptr;
		HRESULT result = m_device->CreateRenderTargetView(m_virtualScreenTexture, &rtvDesc, m_virtualScreenTextureRTV.GetMutablePtr());
		if (result != S_OK)
		{
			if (logger)
				logger->Printf(IGpLogDriver::Category_Error, "GpDisplayDriverD3D11::InitBackBuffer: CreateRenderTargetView for virtual screen texture failed with code %lx", result);

			return false;
		}
	}

	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = vbbFormat;
		srvDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;

		m_virtualScreenTextureSRV = nullptr;
		HRESULT result = m_device->CreateShaderResourceView(m_virtualScreenTexture, &srvDesc, m_virtualScreenTextureSRV.GetMutablePtr());
		if (result != S_OK)
		{
			if (logger)
				logger->Printf(IGpLogDriver::Category_Error, "GpDisplayDriverD3D11::InitBackBuffer: CreateRenderTargetView for virtual screen texture failed with code %lx", result);

			return false;
		}
	}

	return true;
}

bool GpDisplayDriverD3D11::InitResources(uint32_t virtualWidth, uint32_t virtualHeight)
{
	IGpLogDriver *logger = m_properties.m_logger;

	if (logger)
		logger->Printf(IGpLogDriver::Category_Information, "GpDisplayDriverD3D11::InitResources");

	if (!InitBackBuffer(virtualWidth, virtualHeight))
		return false;

	// Quad vertex constant buffer
	{
		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.ByteWidth = sizeof(DrawQuadVertexConstants);
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;

		HRESULT result = m_device->CreateBuffer(&bufferDesc, nullptr, m_drawQuadVertexConstantBuffer.GetMutablePtr());
		if (result != S_OK)
		{
			if (logger)
				logger->Printf(IGpLogDriver::Category_Error, "GpDisplayDriverD3D11::InitResources: CreateBuffer for draw quad vertex constant buffer failed with code %lx", result);

			return false;
		}
	}

	// Quad pixel constant buffer
	{
		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.ByteWidth = sizeof(DrawQuadPixelConstants);
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;

		HRESULT result = m_device->CreateBuffer(&bufferDesc, nullptr, m_drawQuadPixelConstantBuffer.GetMutablePtr());
		if (result != S_OK)
		{
			if (logger)
				logger->Printf(IGpLogDriver::Category_Error, "GpDisplayDriverD3D11::InitResources: CreateBuffer for draw quad pixel constant buffer failed with code %lx", result);

			return false;
		}
	}

	// Quad index buffer
	{
		const uint16_t indexBufferData[] = { 0, 1, 2, 1, 3, 2 };

		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.ByteWidth = sizeof(indexBufferData);
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA initialData;
		initialData.pSysMem = indexBufferData;
		initialData.SysMemPitch = 0;
		initialData.SysMemSlicePitch = 0;

		HRESULT result = m_device->CreateBuffer(&bufferDesc, &initialData, m_quadIndexBuffer.GetMutablePtr());
		if (result != S_OK)
		{
			if (logger)
				logger->Printf(IGpLogDriver::Category_Error, "GpDisplayDriverD3D11::InitResources: CreateBuffer for draw quad index buffer failed with code %lx", result);

			return false;
		}
	}

	// Quad vertex buffer
	{
		const float vertexBufferData[] =
		{
			0.f, 0.0f,
			1.f, 0.f,
			0.f, 1.f,
			1.f, 1.f,
		};

		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.ByteWidth = sizeof(vertexBufferData);
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA initialData;
		initialData.pSysMem = vertexBufferData;
		initialData.SysMemPitch = 0;
		initialData.SysMemSlicePitch = 0;

		HRESULT result = m_device->CreateBuffer(&bufferDesc, &initialData, m_quadVertexBuffer.GetMutablePtr());
		if (result != S_OK)
		{
			if (logger)
				logger->Printf(IGpLogDriver::Category_Error, "GpDisplayDriverD3D11::InitResources: CreateBuffer for draw quad vertex buffer failed with code %lx", result);

			return false;
		}
	}

	{
		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.ByteWidth = sizeof(ScaleQuadPixelConstants);
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;

		HRESULT result = m_device->CreateBuffer(&bufferDesc, nullptr, m_scaleQuadPixelConstantBuffer.GetMutablePtr());
		if (result != S_OK)
		{
			if (logger)
				logger->Printf(IGpLogDriver::Category_Error, "GpDisplayDriverD3D11::InitResources: CreateBuffer for scale quad pixel constant buffer failed with code %lx", result);

			return false;
		}
	}

	const GpShaderCodeBlob drawQuadVBlob = GetBinarizedShader(GpBinarizedShaders::g_drawQuadV_D3D11);
	const GpShaderCodeBlob drawQuadPalettePBlob = GetBinarizedShader(GpBinarizedShaders::g_drawQuadPaletteP_D3D11);
	const GpShaderCodeBlob drawQuadRGBPBlob = GetBinarizedShader(GpBinarizedShaders::g_drawQuadRGBP_D3D11);
	const GpShaderCodeBlob drawQuad15BitPBlob = GetBinarizedShader(GpBinarizedShaders::g_drawQuad15BitP_D3D11);
	const GpShaderCodeBlob drawQuadPaletteICCPBlob = GetBinarizedShader(GpBinarizedShaders::g_drawQuadPaletteICCP_D3D11);
	const GpShaderCodeBlob drawQuadRGBICCPBlob = GetBinarizedShader(GpBinarizedShaders::g_drawQuadRGBICCP_D3D11);
	const GpShaderCodeBlob drawQuad15BitICCPBlob = GetBinarizedShader(GpBinarizedShaders::g_drawQuad15BitICCP_D3D11);
	const GpShaderCodeBlob scaleQuadPBlob = GetBinarizedShader(GpBinarizedShaders::g_scaleQuadP_D3D11);

	m_device->CreateVertexShader(drawQuadVBlob.m_data, drawQuadVBlob.m_size, nullptr, m_drawQuadVertexShader.GetMutablePtr());
	m_device->CreatePixelShader(drawQuadPalettePBlob.m_data, drawQuadPalettePBlob.m_size, nullptr, m_drawQuadPalettePixelShader.GetMutablePtr());
	m_device->CreatePixelShader(drawQuadRGBPBlob.m_data, drawQuadRGBPBlob.m_size, nullptr, m_drawQuadRGBPixelShader.GetMutablePtr());
	m_device->CreatePixelShader(drawQuad15BitPBlob.m_data, drawQuad15BitPBlob.m_size, nullptr, m_drawQuad15BitPixelShader.GetMutablePtr());
	m_device->CreatePixelShader(drawQuadPaletteICCPBlob.m_data, drawQuadPaletteICCPBlob.m_size, nullptr, m_drawQuadPaletteICCPixelShader.GetMutablePtr());
	m_device->CreatePixelShader(drawQuadRGBICCPBlob.m_data, drawQuadRGBICCPBlob.m_size, nullptr, m_drawQuadRGBICCPixelShader.GetMutablePtr());
	m_device->CreatePixelShader(drawQuad15BitICCPBlob.m_data, drawQuad15BitICCPBlob.m_size, nullptr, m_drawQuad15BitICCPixelShader.GetMutablePtr());
	m_device->CreatePixelShader(scaleQuadPBlob.m_data, scaleQuadPBlob.m_size, nullptr, m_scaleQuadPixelShader.GetMutablePtr());

	// Quad input layout
	{
		D3D11_INPUT_ELEMENT_DESC descs[] =
		{
			"POSITION",						// Semantic name
			0,								// Semantic index
			DXGI_FORMAT_R32G32_FLOAT,	// Format
			0,								// Input slot
			0,								// Aligned byte offset
			D3D11_INPUT_PER_VERTEX_DATA,	// Input slot class
			0								// Instance data step rate
		};

		HRESULT result = m_device->CreateInputLayout(descs, sizeof(descs) / sizeof(descs[0]), drawQuadVBlob.m_data, drawQuadVBlob.m_size, m_drawQuadInputLayout.GetMutablePtr());
		if (result != S_OK)
		{
			if (logger)
				logger->Printf(IGpLogDriver::Category_Error, "GpDisplayDriverD3D11::InitResources: CreateInputLayout for draw quad input failed with code %lx", result);

			return false;
		}
	}

	// Quad depth stencil state
	{
		D3D11_DEPTH_STENCIL_DESC desc;
		desc.DepthEnable = FALSE;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
		desc.StencilEnable = FALSE;
		desc.StencilReadMask = 0;
		desc.StencilWriteMask = 0;
		desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		HRESULT result = m_device->CreateDepthStencilState(&desc, m_drawQuadDepthStencilState.GetMutablePtr());
		if (result != S_OK)
		{
			if (logger)
				logger->Printf(IGpLogDriver::Category_Error, "GpDisplayDriverD3D11::InitResources: CreateDepthStencilState for draw quad with code %lx", result);

			return false;
		}
	}

	// Nearest neighbor sampler desc
	{
		D3D11_SAMPLER_DESC samplerDesc;
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.MipLODBias = 0.f;
		samplerDesc.MaxAnisotropy = 1;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDesc.BorderColor[0] = samplerDesc.BorderColor[1] = samplerDesc.BorderColor[2] = samplerDesc.BorderColor[3] = 1.0f;
		samplerDesc.MinLOD = -FLT_MAX;
		samplerDesc.MaxLOD = FLT_MAX;

		HRESULT result = m_device->CreateSamplerState(&samplerDesc, m_nearestNeighborSamplerState.GetMutablePtr());
		if (result != S_OK)
		{
			if (logger)
				logger->Printf(IGpLogDriver::Category_Error, "GpDisplayDriverD3D11::InitResources: CreateSamplerState for nearest neighbor failed with code %lx", result);

			return false;
		}
	}

	DXGI_FORMAT paletteTextureFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Palette texture
	{
		D3D11_TEXTURE1D_DESC desc;
		desc.Width = 256;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = paletteTextureFormat;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;

		uint8_t initialDataBytes[256][4];
		for (int i = 0; i < 256; i++)
		{
			for (int ch = 0; ch < 4; ch++)
				initialDataBytes[i][ch] = 255;
		}

		D3D11_SUBRESOURCE_DATA initialData;
		initialData.pSysMem = initialDataBytes[0];
		initialData.SysMemPitch = 256 * 4;
		initialData.SysMemSlicePitch = 256 * 4;

		HRESULT result = m_device->CreateTexture1D(&desc, &initialData, m_paletteTexture.GetMutablePtr());
		if (result != S_OK)
		{
			if (logger)
				logger->Printf(IGpLogDriver::Category_Error, "GpDisplayDriverD3D11::InitResources: CreateTexture1D for palette failed with code %lx", result);

			return false;
		}
	}

	// Palette texture SRV
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		desc.Format = paletteTextureFormat;
		desc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE1D;
		desc.Texture1D.MostDetailedMip = 0;
		desc.Texture1D.MipLevels = 1;

		HRESULT result = m_device->CreateShaderResourceView(m_paletteTexture, &desc, m_paletteTextureSRV.GetMutablePtr());
		if (result != S_OK)
		{
			if (logger)
				logger->Printf(IGpLogDriver::Category_Error, "GpDisplayDriverD3D11::InitResources: CreateShaderResourceView for palette failed with code %lx", result);

			return false;
		}
	}

	return true;
}


GpDisplayDriverTickStatus_t GpDisplayDriverD3D11::PresentFrameAndSync()
{
	SynchronizeCursors();

	FLOAT bgColor[4];

	for (int i = 0; i < 4; i++)
		bgColor[i] = m_bgColor[i];

	if (m_bgIsDark)
	{
		for (int i = 0; i < 3; i++)
			bgColor[i] *= 0.25f;
	}

	m_deviceContext->ClearRenderTargetView(m_virtualScreenTextureRTV, bgColor);

	//ID3D11RenderTargetView *const rtv = m_backBufferRTV;
	ID3D11RenderTargetView *const vsRTV = m_virtualScreenTextureRTV;
	m_deviceContext->OMSetRenderTargets(1, &vsRTV, nullptr);

	{
		D3D11_VIEWPORT viewport;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = static_cast<FLOAT>(m_windowWidthVirtual);
		viewport.Height = static_cast<FLOAT>(m_windowHeightVirtual);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		m_deviceContext->RSSetViewports(1, &viewport);
	}

	m_properties.m_renderFunc(m_properties.m_renderFuncContext);

	ScaleVirtualScreen();

	DXGI_PRESENT_PARAMETERS presentParams;

	ZeroMemory(&presentParams, sizeof(presentParams));

	UINT lastPresentCount = 0;

	if (FAILED(m_swapChain->GetLastPresentCount(&lastPresentCount)))
		return GpDisplayDriverTickStatuses::kNonFatalFault;

	if (FAILED(m_swapChain->Present1(1, 0, &presentParams)))
		return GpDisplayDriverTickStatuses::kNonFatalFault;

	//DebugPrintf("r: %i\n", static_cast<int>(r));

	DXGI_FRAME_STATISTICS stats;
	if (FAILED(m_swapChain->GetFrameStatistics(&stats)))
		return GpDisplayDriverTickStatuses::kNonFatalFault;

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
			GpDisplayDriverTickStatus_t tickStatus = m_properties.m_tickFunc(m_properties.m_tickFuncContext, m_vosFiber);
			m_frameTimeAccumulated -= m_frameTimeSliceSize;

			if (tickStatus != GpDisplayDriverTickStatuses::kOK)
				return tickStatus;
		}
	}

	return GpDisplayDriverTickStatuses::kOK;
}

void GpDisplayDriverD3D11::ScaleVirtualScreen()
{
	{
		D3D11_VIEWPORT viewport;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = static_cast<FLOAT>(m_windowWidthPhysical);
		viewport.Height = static_cast<FLOAT>(m_windowHeightPhysical);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		m_deviceContext->RSSetViewports(1, &viewport);
	}


	ID3D11Buffer *vbPtr = m_quadVertexBuffer;
	UINT vbStride = sizeof(float) * 2;
	UINT zero = 0;

	ID3D11RenderTargetView *const rtv = m_backBufferRTV;
	m_deviceContext->OMSetRenderTargets(1, &rtv, nullptr);

	//m_deviceContext->OMSetDepthStencilState(m_drawQuadDepthStencilState, 0);

	{
		const float twoDivWidth = 2.0f / static_cast<float>(m_windowWidthPhysical);
		const float negativeTwoDivHeight = -2.0f / static_cast<float>(m_windowHeightPhysical);

		// Use the scaled virtual width instead of the physical width to correctly handle cases where the window boundary is in the middle of a pixel
		float fWidth = static_cast<float>(m_windowWidthVirtual) * m_pixelScaleX;
		float fHeight = static_cast<float>(m_windowHeightVirtual) * m_pixelScaleY;

		DrawQuadVertexConstants vConstantsData;
		vConstantsData.m_ndcOriginX = twoDivWidth - 1.0f;
		vConstantsData.m_ndcOriginY = negativeTwoDivHeight + 1.0f;
		vConstantsData.m_ndcWidth = fWidth * twoDivWidth;
		vConstantsData.m_ndcHeight = fHeight * negativeTwoDivHeight;

		vConstantsData.m_surfaceDimensionX = static_cast<float>(m_windowWidthVirtual);
		vConstantsData.m_surfaceDimensionY = static_cast<float>(m_windowHeightVirtual);

		D3D11_MAPPED_SUBRESOURCE mappedVConstants;
		if (m_deviceContext->Map(m_drawQuadVertexConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedVConstants) == S_OK)
		{
			memcpy(mappedVConstants.pData, &vConstantsData, sizeof(vConstantsData));
			m_deviceContext->Unmap(m_drawQuadVertexConstantBuffer, 0);
		}

		ScaleQuadPixelConstants pConstantsData;
		pConstantsData.m_dx = static_cast<float>(static_cast<double>(m_windowWidthVirtual) / static_cast<double>(m_windowWidthPhysical));
		pConstantsData.m_dy = static_cast<float>(static_cast<double>(m_windowHeightVirtual) / static_cast<double>(m_windowHeightPhysical));

		D3D11_MAPPED_SUBRESOURCE mappedPConstants;
		if (m_deviceContext->Map(m_scaleQuadPixelConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedPConstants) == S_OK)
		{
			memcpy(mappedPConstants.pData, &pConstantsData, sizeof(pConstantsData));
			m_deviceContext->Unmap(m_scaleQuadPixelConstantBuffer, 0);
		}
	}

	m_deviceContext->IASetVertexBuffers(0, 1, &vbPtr, &vbStride, &zero);
	m_deviceContext->IASetIndexBuffer(m_quadIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_deviceContext->IASetInputLayout(m_drawQuadInputLayout);

	ID3D11Buffer *vsConstants = m_drawQuadVertexConstantBuffer;
	m_deviceContext->VSSetShader(m_drawQuadVertexShader, nullptr, 0);
	m_deviceContext->VSSetConstantBuffers(0, 1, &vsConstants);

	ID3D11SamplerState *samplerStates[] =
	{
		m_nearestNeighborSamplerState,
	};
	m_deviceContext->PSSetSamplers(0, sizeof(samplerStates) / sizeof(samplerStates[0]), samplerStates);


	ID3D11ShaderResourceView *psResourceViews[] =
	{
		m_virtualScreenTextureSRV,
	};

	ID3D11Buffer *psConstants = m_scaleQuadPixelConstantBuffer;

	m_deviceContext->PSSetShader(m_scaleQuadPixelShader, nullptr, 0);
	m_deviceContext->PSSetShaderResources(0, sizeof(psResourceViews) / sizeof(psResourceViews[0]), psResourceViews);
	m_deviceContext->PSSetConstantBuffers(0, 1, &psConstants);

	m_deviceContext->DrawIndexed(6, 0, 0);

	ID3D11ShaderResourceView *unbindPSResourceViews[] =
	{
		0,
	};
	m_deviceContext->PSSetShaderResources(0, sizeof(unbindPSResourceViews) / sizeof(unbindPSResourceViews[0]), unbindPSResourceViews);

	ID3D11Buffer *clearBuffer = nullptr;
	m_deviceContext->PSSetConstantBuffers(0, 1, &clearBuffer);
}

void GpDisplayDriverD3D11::SynchronizeCursors()
{
	HCURSOR replacementCursor = nullptr;

	if (m_activeCursor)
	{
		if (m_pendingCursor != m_activeCursor)
		{
			if (m_pendingCursor == nullptr)
			{
				m_currentStandardCursor = m_pendingStandardCursor;
				ChangeToStandardCursor(m_currentStandardCursor);

				m_activeCursor->DecRef();
				m_activeCursor = nullptr;
			}
			else
			{
				ChangeToCursor(m_pendingCursor->GetHCursor());

				m_pendingCursor->IncRef();
				m_activeCursor->DecRef();
				m_activeCursor = m_pendingCursor;
			}
		}
	}
	else
	{
		if (m_pendingCursor)
		{
			m_pendingCursor->IncRef();
			m_activeCursor = m_pendingCursor;

			ChangeToCursor(m_activeCursor->GetHCursor());
		}
		else
		{
			if (m_pendingStandardCursor != m_currentStandardCursor)
			{
				ChangeToStandardCursor(m_pendingStandardCursor);
				m_currentStandardCursor = m_pendingStandardCursor;
			}
		}
	}
}


void GpDisplayDriverD3D11::ChangeToCursor(HCURSOR cursor)
{
	if (m_mouseIsInClientArea)
		::SetCursor(cursor);

	SetClassLongPtrW(m_osGlobals->m_hwnd, GCLP_HCURSOR, reinterpret_cast<LONG_PTR>(cursor));
}

void GpDisplayDriverD3D11::ChangeToStandardCursor(EGpStandardCursor_t cursor)
{
	switch (cursor)
	{
	case EGpStandardCursors::kIBeam:
		ChangeToCursor(m_ibeamCursor);
		break;
	case EGpStandardCursors::kWait:
		ChangeToCursor(m_waitCursor);
		break;
	case EGpStandardCursors::kArrow:
	default:
		ChangeToCursor(m_arrowCursor);
		break;
	}
}

void GpDisplayDriverD3D11::BecomeFullScreen(LONG &windowStyle)
{
	IGpLogDriver *logger = m_properties.m_logger;

	if (logger)
		logger->Printf(IGpLogDriver::Category_Information, "GpDisplayDriverD3D11::BecomeFullScreen");

	assert(!m_isFullScreen);

	RECT windowRect;
	if (!GetWindowRect(m_osGlobals->m_hwnd, &windowRect))
	{
		if (logger)
			logger->Printf(IGpLogDriver::Category_Error, "GpDisplayDriverD3D11::BecomeFullScreen: GetWindowRect failed");

		return;	// ???
	}

	HMONITOR monitor = MonitorFromRect(&windowRect, MONITOR_DEFAULTTONULL);
	if (!monitor)
	{
		// If the window is off-screen, use the primary monitor
		monitor = MonitorFromRect(&windowRect, MONITOR_DEFAULTTOPRIMARY);
	}
	else
	{
		// Otherwise, use the nearest
		monitor = MonitorFromRect(&windowRect, MONITOR_DEFAULTTONEAREST);
	}

	if (!monitor)
	{
		if (logger)
			logger->Printf(IGpLogDriver::Category_Error, "GpDisplayDriverD3D11::BecomeFullScreen: Couldn't find any monitors");

		return;	// No monitor?
	}

	MONITORINFO monitorInfo;
	monitorInfo.cbSize = sizeof(monitorInfo);
	if (!GetMonitorInfoA(monitor, &monitorInfo))
	{
		if (logger)
			logger->Printf(IGpLogDriver::Category_Error, "GpDisplayDriverD3D11::BecomeFullScreen: GetMonitorInfoA failed");

		return;
	}

	m_windowModeRevertRect = windowRect;
	SetWindowLongPtr(m_osGlobals->m_hwnd, GWL_STYLE, WS_VISIBLE | WS_POPUP);

	if (!SetWindowPos(m_osGlobals->m_hwnd, HWND_TOP, monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top, monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top, SWP_FRAMECHANGED))
	{
		if (logger)
			logger->Printf(IGpLogDriver::Category_Error, "GpDisplayDriverD3D11::BecomeFullScreen: SetWindowPos failed");
	}

	m_isFullScreen = true;
	windowStyle = (WS_VISIBLE | WS_POPUP);
}

void GpDisplayDriverD3D11::BecomeWindowed(LONG &windowStyle)
{
	IGpLogDriver *logger = m_properties.m_logger;

	if (logger)
		logger->Printf(IGpLogDriver::Category_Information, "GpDisplayDriverD3D11::BecomeWindowed");

	assert(m_isFullScreen);

	RECT revertRect = m_windowModeRevertRect;

	HMONITOR monitor = MonitorFromRect(&m_windowModeRevertRect, MONITOR_DEFAULTTONULL);
	if (!monitor)
	{
		// If the window is off-screen, use the primary monitor
		monitor = MonitorFromRect(&revertRect, MONITOR_DEFAULTTOPRIMARY);
		if (!monitor)
		{
			if (logger)
				logger->Printf(IGpLogDriver::Category_Error, "GpDisplayDriverD3D11::BecomeWindowed: MonitorFromRect fallback failed");

			return;
		}

		MONITORINFO monitorInfo;
		monitorInfo.cbSize = sizeof(monitorInfo);
		if (!GetMonitorInfoA(monitor, &monitorInfo))
		{
			if (logger)
				logger->Printf(IGpLogDriver::Category_Error, "GpDisplayDriverD3D11::BecomeWindowed: GetMonitorInfoA failed");

			return;
		}

		RECT monitorRect = monitorInfo.rcWork;
		LONG monitorWidth = monitorRect.right - monitorRect.left;
		LONG monitorHeight = monitorRect.bottom - monitorRect.top;

		LONG revertHeight = revertRect.bottom - revertRect.top;
		LONG revertWidth = revertRect.right - revertRect.left;

		if (revertWidth > monitorWidth)
			revertWidth = monitorWidth;

		if (revertHeight > monitorHeight)
			revertHeight = monitorHeight;

		revertRect.bottom = revertRect.top + revertHeight;
		revertRect.right = revertRect.right + revertWidth;

		LONG xDelta = 0;
		if (revertRect.right > monitorRect.right)
			xDelta = monitorRect.right - revertRect.right;
		else if (revertRect.left < monitorRect.left)
			xDelta = monitorRect.left - revertRect.left;

		LONG yDelta = 0;
		if (revertRect.bottom > monitorRect.bottom)
			yDelta = monitorRect.bottom - revertRect.bottom;
		else if (revertRect.top < monitorRect.top)
			yDelta = monitorRect.top - revertRect.top;


		revertRect.left = revertRect.left + xDelta;
		revertRect.top = revertRect.top + yDelta;
		revertRect.bottom = revertRect.top + revertHeight;
		revertRect.right = revertRect.right + revertWidth;
	}

	SetWindowLongPtrW(m_osGlobals->m_hwnd, GWL_STYLE, WS_VISIBLE | WS_OVERLAPPEDWINDOW);

	if (!SetWindowPos(m_osGlobals->m_hwnd, HWND_TOP, revertRect.left, revertRect.top, revertRect.right - revertRect.left, revertRect.bottom - revertRect.top, SWP_FRAMECHANGED))
	{
		if (logger)
			logger->Printf(IGpLogDriver::Category_Error, "GpDisplayDriverD3D11::BecomeWindowed: SetWindowPos failed");
	}

	m_isFullScreen = false;
	windowStyle = (WS_VISIBLE | WS_OVERLAPPEDWINDOW);
}

void GpDisplayDriverD3D11::Run()
{
	IGpLogDriver *logger = m_properties.m_logger;

	WNDCLASSEX wc;

	LPVOID fiber = ConvertThreadToFiberEx(this, 0);
	if (!fiber)
	{
		if (logger)
			logger->Printf(IGpLogDriver::Category_Error, "ConvertThreadToFiberEx failed");

		return;	// ???
	}

	m_vosFiber = m_osGlobals->m_createFiberFunc(fiber);

	ZeroMemory(&wc, sizeof(wc));

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WinProc;
	wc.hInstance = m_osGlobals->m_hInstance;
	wc.hCursor = m_arrowCursor;
	wc.hIcon = m_osGlobals->m_hIcon;
	wc.hIconSm = m_osGlobals->m_hIconSm;
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName = "GPD3D11WindowClass";

	RegisterClassEx(&wc);

	LONG windowStyle = WS_OVERLAPPEDWINDOW;
	HMENU menus = NULL;

	// TODO: Fix the resolution here
	RECT wr = { 0, 0, m_windowWidthPhysical, m_windowHeightPhysical };
	AdjustWindowRect(&wr, windowStyle, menus != NULL);

	m_osGlobals->m_hwnd = CreateWindowExW(NULL, L"GPD3D11WindowClass", GP_APPLICATION_NAME_W, WS_OVERLAPPEDWINDOW, 300, 300, wr.right - wr.left, wr.bottom - wr.top, NULL, menus, m_osGlobals->m_hInstance, NULL);

	ShowWindow(m_osGlobals->m_hwnd, m_osGlobals->m_nCmdShow);

	StartD3DForWindow(m_osGlobals->m_hwnd, m_swapChain, m_device, m_deviceContext, logger);

	InitResources(m_windowWidthVirtual, m_windowHeightVirtual);

	LARGE_INTEGER lastTimestamp;
	memset(&lastTimestamp, 0, sizeof(lastTimestamp));

	MSG msg;
	for (;;)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			DispatchMessage(&msg);

			{
				if (msg.message == WM_MOUSEMOVE)
				{
					if (!m_mouseIsInClientArea)
					{
						m_mouseIsInClientArea = true;

						TRACKMOUSEEVENT tme;
						ZeroMemory(&tme, sizeof(tme));

						tme.cbSize = sizeof(tme);
						tme.dwFlags = TME_LEAVE;
						tme.hwndTrack = m_osGlobals->m_hwnd;
						tme.dwHoverTime = HOVER_DEFAULT;
						TrackMouseEvent(&tme);
					}
				}
				else if (msg.message == WM_MOUSELEAVE)
					m_mouseIsInClientArea = false;

				m_osGlobals->m_translateWindowsMessageFunc(&msg, m_properties.m_eventQueue, m_pixelScaleX, m_pixelScaleY);
			}
		}
		else
		{
			if (m_isFullScreen != m_isFullScreenDesired)
			{
				if (m_isFullScreenDesired)
					BecomeFullScreen(windowStyle);
				else
					BecomeWindowed(windowStyle);
			}

			RECT clientRect;
			GetClientRect(m_osGlobals->m_hwnd, &clientRect);

			unsigned int desiredWidth = clientRect.right - clientRect.left;
			unsigned int desiredHeight = clientRect.bottom - clientRect.top;
			if (clientRect.right - clientRect.left != m_windowWidthPhysical || clientRect.bottom - clientRect.top != m_windowHeightPhysical || m_isResolutionResetDesired)
			{
				uint32_t prevWidthPhysical = m_windowWidthPhysical;
				uint32_t prevHeightPhysical = m_windowHeightPhysical;
				uint32_t prevWidthVirtual = m_windowWidthVirtual;
				uint32_t prevHeightVirtual = m_windowHeightVirtual;
				uint32_t virtualWidth = m_windowWidthVirtual;
				uint32_t virtualHeight = m_windowHeightVirtual;
				float pixelScaleX = 1.0f;
				float pixelScaleY = 1.0f;

				if (m_properties.m_adjustRequestedResolutionFunc(m_properties.m_adjustRequestedResolutionFuncContext, desiredWidth, desiredHeight, virtualWidth, virtualHeight, pixelScaleX, pixelScaleY))
				{
					bool resizedOK = ResizeD3DWindow(m_osGlobals->m_hwnd, m_windowWidthPhysical, m_windowHeightPhysical, desiredWidth, desiredHeight, windowStyle, menus, logger);
					resizedOK = resizedOK && DetachSwapChain();
					resizedOK = resizedOK && ResizeSwapChain(m_swapChain, m_windowWidthPhysical, m_windowHeightPhysical);
					resizedOK = resizedOK && InitBackBuffer(virtualWidth, virtualHeight);

					if (!resizedOK)
						break;	// Critical video driver error, exit

					m_windowWidthVirtual = virtualWidth;
					m_windowHeightVirtual = virtualHeight;
					m_pixelScaleX = pixelScaleX;
					m_pixelScaleY = pixelScaleY;
					m_isResolutionResetDesired = false;

					if (GpVOSEvent *resizeEvent = m_properties.m_eventQueue->QueueEvent())
					{
						resizeEvent->m_eventType = GpVOSEventTypes::kVideoResolutionChanged;
						resizeEvent->m_event.m_resolutionChangedEvent.m_prevWidth = prevWidthVirtual;
						resizeEvent->m_event.m_resolutionChangedEvent.m_prevHeight = prevHeightVirtual;
						resizeEvent->m_event.m_resolutionChangedEvent.m_newWidth = m_windowWidthVirtual;
						resizeEvent->m_event.m_resolutionChangedEvent.m_newHeight = m_windowHeightVirtual;
					}
				}
			}

			GpDisplayDriverTickStatus_t tickStatus = PresentFrameAndSync();
			if (tickStatus == GpDisplayDriverTickStatuses::kFatalFault || tickStatus == GpDisplayDriverTickStatuses::kApplicationTerminated)
				break;
		}
	}

	// Exit
	ConvertFiberToThread();
}

void GpDisplayDriverD3D11::Shutdown()
{
	this->~GpDisplayDriverD3D11();
	free(this);
}

void GpDisplayDriverD3D11::GetDisplayResolution(unsigned int *width, unsigned int *height)
{
	if (width)
		*width = m_windowWidthVirtual;
	if (height)
		*height = m_windowHeightVirtual;
}

IGpDisplayDriverSurface *GpDisplayDriverD3D11::CreateSurface(size_t width, size_t height, GpPixelFormat_t pixelFormat)
{
	return GpDisplayDriverSurfaceD3D11::Create(m_device, m_deviceContext, width, height, pixelFormat);
}

void GpDisplayDriverD3D11::DrawSurface(IGpDisplayDriverSurface *surface, int32_t x, int32_t y, size_t width, size_t height, const GpDisplayDriverSurfaceEffects *effects)
{
	if (!effects)
		effects = &gs_defaultEffects;

	ID3D11Buffer *vbPtr = m_quadVertexBuffer;
	UINT vbStride = sizeof(float) * 2;
	UINT zero = 0;

	GpDisplayDriverSurfaceD3D11 *d3d11Surface = static_cast<GpDisplayDriverSurfaceD3D11*>(surface);

	//m_deviceContext->OMSetDepthStencilState(m_drawQuadDepthStencilState, 0);

	{
		const float twoDivWidth = 2.0f / static_cast<float>(m_windowWidthVirtual);
		const float negativeTwoDivHeight = -2.0f / static_cast<float>(m_windowHeightVirtual);

		DrawQuadVertexConstants vConstantsData;
		vConstantsData.m_ndcOriginX = static_cast<float>(x) * twoDivWidth - 1.0f;
		vConstantsData.m_ndcOriginY = static_cast<float>(y) * negativeTwoDivHeight + 1.0f;
		vConstantsData.m_ndcWidth = static_cast<float>(width) * twoDivWidth;
		vConstantsData.m_ndcHeight = static_cast<float>(height) * negativeTwoDivHeight;

		vConstantsData.m_surfaceDimensionX = static_cast<float>(d3d11Surface->GetWidth());
		vConstantsData.m_surfaceDimensionY = static_cast<float>(d3d11Surface->GetHeight());

		D3D11_MAPPED_SUBRESOURCE vMappedConstants;
		if (m_deviceContext->Map(m_drawQuadVertexConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &vMappedConstants) == S_OK)
		{
			memcpy(vMappedConstants.pData, &vConstantsData, sizeof(vConstantsData));
			m_deviceContext->Unmap(m_drawQuadVertexConstantBuffer, 0);
		}

		DrawQuadPixelConstants pConstantsData;
		for (int i = 0; i < 4; i++)
			pConstantsData.m_modulation[i] = 1.0f;

		if (effects->m_flicker)
		{
			pConstantsData.m_flickerAxis[0] = effects->m_flickerAxisX;
			pConstantsData.m_flickerAxis[1] = effects->m_flickerAxisY;
			pConstantsData.m_flickerStart = effects->m_flickerStartThreshold;
			pConstantsData.m_flickerEnd = effects->m_flickerEndThreshold;
		}
		else
		{
			pConstantsData.m_flickerAxis[0] = pConstantsData.m_flickerAxis[1] = 0;
			pConstantsData.m_flickerEnd = -1;
			pConstantsData.m_flickerStart = -2;
		}

		pConstantsData.m_desaturation = effects->m_desaturation;

		if (effects->m_darken)
			for (int i = 0; i < 3; i++)
				pConstantsData.m_modulation[i] = 0.5f;

		D3D11_MAPPED_SUBRESOURCE pMappedConstants;
		if (m_deviceContext->Map(m_drawQuadPixelConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &pMappedConstants) == S_OK)
		{
			memcpy(pMappedConstants.pData, &pConstantsData, sizeof(pConstantsData));
			m_deviceContext->Unmap(m_drawQuadPixelConstantBuffer, 0);
		}
	}

	m_deviceContext->IASetVertexBuffers(0, 1, &vbPtr, &vbStride, &zero);
	m_deviceContext->IASetIndexBuffer(m_quadIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_deviceContext->IASetInputLayout(m_drawQuadInputLayout);

	ID3D11Buffer *vsConstants = m_drawQuadVertexConstantBuffer;
	m_deviceContext->VSSetShader(m_drawQuadVertexShader, nullptr, 0);
	m_deviceContext->VSSetConstantBuffers(0, 1, &vsConstants);

	ID3D11SamplerState *samplerStates[] =
	{
		m_nearestNeighborSamplerState,
	};
	m_deviceContext->PSSetSamplers(0, sizeof(samplerStates) / sizeof(samplerStates[0]), samplerStates);

	ID3D11Buffer *psConstants = m_drawQuadPixelConstantBuffer;
	m_deviceContext->PSSetConstantBuffers(0, 1, &psConstants);

	GpPixelFormat_t pixelFormat = d3d11Surface->GetPixelFormat();
	if (pixelFormat == GpPixelFormats::k8BitStandard || pixelFormat == GpPixelFormats::k8BitCustom)
	{
		ID3D11ShaderResourceView *psResourceViews[] =
		{
			d3d11Surface->GetSRV(),
			m_paletteTextureSRV
		};

		m_deviceContext->PSSetShader(m_useICCProfile ? m_drawQuadPaletteICCPixelShader : m_drawQuadPalettePixelShader, nullptr, 0);
		m_deviceContext->PSSetShaderResources(0, sizeof(psResourceViews) / sizeof(psResourceViews[0]), psResourceViews);
	}
	else if (pixelFormat == GpPixelFormats::kRGB555)
	{
		ID3D11ShaderResourceView *psResourceViews[] =
		{
			d3d11Surface->GetSRV(),
		};

		m_deviceContext->PSSetShader(m_useICCProfile ? m_drawQuad15BitICCPixelShader : m_drawQuad15BitPixelShader, nullptr, 0);
		m_deviceContext->PSSetShaderResources(0, sizeof(psResourceViews) / sizeof(psResourceViews[0]), psResourceViews);
	}
	else if (pixelFormat == GpPixelFormats::kRGB32)
	{
		ID3D11ShaderResourceView *psResourceViews[] =
		{
			d3d11Surface->GetSRV(),
		};

		m_deviceContext->PSSetShader(m_useICCProfile ? m_drawQuadRGBICCPixelShader : m_drawQuadRGBPixelShader, nullptr, 0);
		m_deviceContext->PSSetShaderResources(0, sizeof(psResourceViews) / sizeof(psResourceViews[0]), psResourceViews);
	}
	else
	{
		return;
	}

	m_deviceContext->DrawIndexed(6, 0, 0);
}

IGpCursor *GpDisplayDriverD3D11::LoadCursor(bool isColor, int cursorID)
{
	const size_t bufSize = MAX_PATH;
	wchar_t path[bufSize];

	int sz = 0;
	if (isColor)
		sz = _snwprintf(path, bufSize, L"%sPackaged\\WinCursors\\c%i.cur", m_osGlobals->m_baseDir, cursorID);
	else
		sz = _snwprintf(path, bufSize, L"%sPackaged\\WinCursors\\b%i.cur", m_osGlobals->m_baseDir, cursorID);

	if (sz < 0 || static_cast<size_t>(sz) >= bufSize)
		return nullptr;

	return m_osGlobals->m_loadCursorFunc(path);
}


// We can't just set the cursor because we want to post WM_SETCURSOR to keep it limited
// to the game window area, but depending on the fiber implementation, this may not be
// the window thread.
void GpDisplayDriverD3D11::SetCursor(IGpCursor *cursor)
{
	IGpCursor_Win32 *winCursor = static_cast<IGpCursor_Win32*>(cursor);

	winCursor->IncRef();

	if (m_pendingCursor)
		m_pendingCursor->DecRef();

	m_pendingCursor = winCursor;
}

void GpDisplayDriverD3D11::SetStandardCursor(EGpStandardCursor_t standardCursor)
{
	if (m_pendingCursor)
	{
		m_pendingCursor->DecRef();
		m_pendingCursor = nullptr;
	}

	m_pendingStandardCursor = standardCursor;
}

void GpDisplayDriverD3D11::UpdatePalette(const void *paletteData)
{
	const size_t dataSize = 256 * 4;
	const uint8_t *paletteDataBytes = static_cast<const uint8_t *>(paletteData);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (m_deviceContext->Map(m_paletteTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource) == S_OK)
	{
		uint8_t *textureDataBytes = static_cast<uint8_t *>(mappedResource.pData);

		for (size_t chunkOffset = 0; chunkOffset < dataSize; chunkOffset += sizeof(__m128i))
		{
			__m128i chunkData = _mm_loadu_si128(reinterpret_cast<const __m128i*>(paletteDataBytes + chunkOffset));
			_mm_stream_si128(reinterpret_cast<__m128i*>(textureDataBytes + chunkOffset), chunkData);
		}

		m_deviceContext->Unmap(m_paletteTexture, 0);
	}
}
void GpDisplayDriverD3D11::SetBackgroundColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	m_bgColor[0] = static_cast<float>(r) / 255.0f;
	m_bgColor[1] = static_cast<float>(g) / 255.0f;
	m_bgColor[2] = static_cast<float>(b) / 255.0f;
	m_bgColor[3] = static_cast<float>(a) / 255.0f;
}

void GpDisplayDriverD3D11::SetBackgroundDarkenEffect(bool isDark)
{
	m_bgIsDark = isDark;
}


void GpDisplayDriverD3D11::SetUseICCProfile(bool useICCProfile)
{
	m_useICCProfile = useICCProfile;
}

void GpDisplayDriverD3D11::RequestToggleFullScreen(uint32_t timestamp)
{
	// Alt-Enter gets re-sent after a full-screen toggle, so we ignore toggle requests until half a second has elapsed
	if (timestamp == 0 || timestamp > m_lastFullScreenToggleTimeStamp + 30)
	{
		m_isFullScreenDesired = !m_isFullScreenDesired;
		m_lastFullScreenToggleTimeStamp = timestamp;
	}
}

void GpDisplayDriverD3D11::RequestResetVirtualResolution()
{
	m_isResolutionResetDesired = true;
}

bool GpDisplayDriverD3D11::IsFullScreen() const
{
	return m_isFullScreenDesired;
}

const GpDisplayDriverProperties &GpDisplayDriverD3D11::GetProperties() const
{
	return m_properties;
}

IGpPrefsHandler *GpDisplayDriverD3D11::GetPrefsHandler() const
{
	const IGpPrefsHandler *cPrefsHandler = this;
	return const_cast<IGpPrefsHandler*>(cPrefsHandler);
}

void GpDisplayDriverD3D11::ApplyPrefs(const void *identifier, size_t identifierSize, const void *contents, size_t contentsSize, uint32_t version)
{
	if (version == kPrefsVersion && identifierSize == strlen(kPrefsIdentifier) && !memcmp(identifier, kPrefsIdentifier, identifierSize))
	{
		const GpDisplayDriverD3D11_Prefs *prefs = static_cast<const GpDisplayDriverD3D11_Prefs *>(contents);
		m_isFullScreenDesired = prefs->m_isFullScreen;
	}
}

bool GpDisplayDriverD3D11::SavePrefs(void *context, IGpPrefsHandler::WritePrefsFunc_t writeFunc)
{
	GpDisplayDriverD3D11_Prefs prefs;
	prefs.m_isFullScreen = m_isFullScreenDesired;

	return writeFunc(context, kPrefsIdentifier, strlen(kPrefsIdentifier), &prefs, sizeof(prefs), kPrefsVersion);
}

GpDisplayDriverD3D11 *GpDisplayDriverD3D11::Create(const GpDisplayDriverProperties &properties)
{
	void *storage = malloc(sizeof(GpDisplayDriverD3D11));
	if (!storage)
		return nullptr;

	return new (storage) GpDisplayDriverD3D11(properties);
}

GpDisplayDriverD3D11::GpDisplayDriverD3D11(const GpDisplayDriverProperties &properties)
	: m_properties(properties)
	, m_frameTimeAccumulated(0)
	, m_windowWidthPhysical(640)
	, m_windowHeightPhysical(480)
	, m_windowWidthVirtual(640)
	, m_windowHeightVirtual(480)
	, m_pixelScaleX(1.0f)
	, m_pixelScaleY(1.0f)
	, m_vosFiber(nullptr)
	, m_osGlobals(static_cast<GpWindowsGlobals*>(properties.m_osGlobals))
	, m_pendingCursor(nullptr)
	, m_activeCursor(nullptr)
	, m_currentStandardCursor(EGpStandardCursors::kArrow)
	, m_pendingStandardCursor(EGpStandardCursors::kArrow)
	, m_mouseIsInClientArea(false)
	, m_isFullScreen(false)
	, m_isFullScreenDesired(false)
	, m_isResolutionResetDesired(false)
	, m_lastFullScreenToggleTimeStamp(0)
	, m_bgIsDark(false)
	, m_useICCProfile(false)
{
	memset(&m_syncTimeBase, 0, sizeof(m_syncTimeBase));
	memset(&m_windowModeRevertRect, 0, sizeof(m_windowModeRevertRect));

	QueryPerformanceFrequency(&m_QPFrequency);

	m_frameTimeSliceSize = m_QPFrequency.QuadPart * static_cast<LONGLONG>(properties.m_frameTimeLockNumerator) / static_cast<LONGLONG>(properties.m_frameTimeLockDenominator);

	m_arrowCursor = reinterpret_cast<HCURSOR>(LoadImageW(nullptr, MAKEINTRESOURCEW(OCR_NORMAL), IMAGE_CURSOR, 0, 0, LR_SHARED));
	m_ibeamCursor = reinterpret_cast<HCURSOR>(LoadImageW(nullptr, MAKEINTRESOURCEW(OCR_IBEAM), IMAGE_CURSOR, 0, 0, LR_SHARED));
	m_waitCursor = reinterpret_cast<HCURSOR>(LoadImageW(nullptr, MAKEINTRESOURCEW(OCR_WAIT), IMAGE_CURSOR, 0, 0, LR_SHARED));

	m_bgColor[0] = 0.0f;
	m_bgColor[1] = 0.0f;
	m_bgColor[2] = 0.0f;
	m_bgColor[3] = 1.0f;
}

GpDisplayDriverD3D11::~GpDisplayDriverD3D11()
{
	// GP TODO: Sloppy cleanup... Close the window!!
}

extern "C" __declspec(dllexport) IGpDisplayDriver *GpDriver_CreateDisplayDriver_D3D11(const GpDisplayDriverProperties &properties)
{
	return GpDisplayDriverD3D11::Create(properties);
}
