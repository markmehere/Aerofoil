#pragma once

#include "IGpDisplayDriverSurface.h"
#include "GpComPtr.h"
#include "GpPixelFormat.h"

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11ShaderResourceView;
struct ID3D11Texture2D;
struct IGpAllocator;

class GpDisplayDriverSurfaceD3D11 final : public IGpDisplayDriverSurface
{
public:
	virtual void Upload(const void *data, size_t x, size_t y, size_t width, size_t height, size_t pitch) override;
	virtual void UploadEntire(const void *data, size_t pitch) override;
	virtual void Destroy() override;

	ID3D11ShaderResourceView *GetSRV() const;
	GpPixelFormat_t GetPixelFormat() const;
	size_t GetWidth() const;
	size_t GetHeight() const;

	static GpDisplayDriverSurfaceD3D11 *Create(ID3D11Device *device, ID3D11DeviceContext *deviceContext, size_t width, size_t height, GpPixelFormat_t pixelFormat, IGpAllocator *alloc);

private:
	GpDisplayDriverSurfaceD3D11(ID3D11Device *device, ID3D11DeviceContext *deviceContext, ID3D11Texture2D *texture, ID3D11ShaderResourceView *srv, size_t width, size_t height, GpPixelFormat_t pixelFormat, IGpAllocator *alloc);
	~GpDisplayDriverSurfaceD3D11();

	size_t m_width;
	size_t m_height;
	GpPixelFormat_t m_pixelFormat;
	ID3D11Device *m_device;
	ID3D11DeviceContext *m_deviceContext;
	IGpAllocator *m_alloc;

	GpComPtr<ID3D11Texture2D> m_texture;
	GpComPtr<ID3D11ShaderResourceView> m_srv;
};
