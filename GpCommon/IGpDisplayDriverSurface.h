#pragma once

struct IGpDisplayDriverSurface
{
	virtual void Upload(const void *data, size_t x, size_t y, size_t width, size_t height, size_t pitch) = 0;
	virtual void UploadEntire(const void *data, size_t pitch) = 0;
	virtual void Destroy() = 0;
};
