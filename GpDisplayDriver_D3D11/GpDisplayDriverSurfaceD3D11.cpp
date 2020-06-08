#include "GpDisplayDriverSurfaceD3D11.h"
#include "GpComPtr.h"

#include <d3d11.h>
#include <emmintrin.h>

#include <stdint.h>
#include <stdlib.h>
#include <new>

void GpDisplayDriverSurfaceD3D11::Upload(const void *data, size_t x, size_t y, size_t width, size_t height, size_t pitch)
{
	D3D11_BOX box;
	box.left = x;
	box.right = x + width;
	box.top = y;
	box.bottom = y + height;
	box.front = 0;
	box.back = 1;

	m_deviceContext->UpdateSubresource(m_texture, 0, &box, data, pitch, height * pitch);
}

void GpDisplayDriverSurfaceD3D11::UploadEntire(const void *data, size_t pitch)
{
	D3D11_MAPPED_SUBRESOURCE mappedRes;
	if (m_deviceContext->Map(m_texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes) == S_OK)
	{
		if (mappedRes.RowPitch < pitch || (pitch % 16) != 0)
			return;		// This should not happen, D3D10+ feature level should guarantee 16-byte alignment

		const size_t destPitch = mappedRes.RowPitch;

		const uint8_t *srcBytes = static_cast<const uint8_t*>(data);
		uint8_t *destBytes = static_cast<uint8_t*>(mappedRes.pData);

		const size_t height = m_height;
		const size_t numM128PerRow = pitch / 16;

		for (size_t row = 0; row < height; row++)
		{
			for (size_t moffs = 0; moffs < numM128PerRow; moffs++)
			{
				const __m128i v = _mm_load_si128(reinterpret_cast<const __m128i*>(srcBytes + moffs * 16));
				_mm_stream_si128(reinterpret_cast<__m128i*>(destBytes + moffs * 16), v);
			}

			srcBytes += pitch;
			destBytes += destPitch;
		}

		m_deviceContext->Unmap(m_texture, 0);
	}
}

void GpDisplayDriverSurfaceD3D11::Destroy()
{
	this->~GpDisplayDriverSurfaceD3D11();
	free(this);
}

ID3D11ShaderResourceView *GpDisplayDriverSurfaceD3D11::GetSRV() const
{
	return m_srv;
}

GpPixelFormat_t GpDisplayDriverSurfaceD3D11::GetPixelFormat() const
{
	return m_pixelFormat;
}

size_t GpDisplayDriverSurfaceD3D11::GetWidth() const
{
	return m_width;
}

size_t GpDisplayDriverSurfaceD3D11::GetHeight() const
{
	return m_height;
}

GpDisplayDriverSurfaceD3D11 *GpDisplayDriverSurfaceD3D11::Create(ID3D11Device *device, ID3D11DeviceContext *deviceContext, size_t width, size_t height, GpPixelFormat_t pixelFormat)
{
	DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R8_UNORM;

	switch (pixelFormat)
	{
	case GpPixelFormats::k8BitCustom:
	case GpPixelFormats::k8BitStandard:
		dxgiFormat = DXGI_FORMAT_R8_UINT;
		break;
	case GpPixelFormats::kRGB555:
		dxgiFormat = DXGI_FORMAT_R16_UINT;
		break;
	case GpPixelFormats::kRGB32:
		dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		break;
	case GpPixelFormats::kRGB24:	// RGB24 is not supported as a surface format (PL must convert it)
	default:
		return nullptr;
	}

	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = static_cast<UINT>(width);
	textureDesc.Height = static_cast<UINT>(height);
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = dxgiFormat;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DYNAMIC;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	textureDesc.MiscFlags = 0;

	GpComPtr<ID3D11Texture2D> texture;
	if (device->CreateTexture2D(&textureDesc, nullptr, texture.GetMutablePtr()) != S_OK)
		return nullptr;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = dxgiFormat;
	srvDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;

	GpComPtr<ID3D11ShaderResourceView> srv;
	if (device->CreateShaderResourceView(texture, &srvDesc, srv.GetMutablePtr()) != S_OK)
		return nullptr;

	void *storage = malloc(sizeof(GpDisplayDriverSurfaceD3D11));
	if (!storage)
	{
		texture->Release();
		return nullptr;
	}

	return new (storage) GpDisplayDriverSurfaceD3D11(device, deviceContext, texture, srv, width, height, pixelFormat);
}

GpDisplayDriverSurfaceD3D11::GpDisplayDriverSurfaceD3D11(ID3D11Device *device, ID3D11DeviceContext *deviceContext, ID3D11Texture2D *texture, ID3D11ShaderResourceView *srv, size_t width, size_t height, GpPixelFormat_t pixelFormat)
	: m_width(width)
	, m_height(height)
	, m_pixelFormat(pixelFormat)
	, m_device(device)
	, m_deviceContext(deviceContext)
	, m_texture(texture)
	, m_srv(srv)
{
}

GpDisplayDriverSurfaceD3D11::~GpDisplayDriverSurfaceD3D11()
{
}
