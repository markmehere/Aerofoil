#include "QDGraf.h"

#include "MemoryManager.h"
#include "QDPixMap.h"
#include "QDPort.h"
#include "IGpDisplayDriver.h"
#include "IGpDisplayDriverSurface.h"

DrawSurface::~DrawSurface()
{
}

void DrawSurface::PushToDDSurface(IGpDisplayDriver *displayDriver)
{
	const PixMap *pixMap = *m_port.GetPixMap();
	const size_t width = pixMap->m_rect.right - pixMap->m_rect.left;
	const size_t height = pixMap->m_rect.bottom - pixMap->m_rect.top;

	if (m_port.IsDirty(PortabilityLayer::QDPortDirtyFlag_Size))
	{
		if (m_ddSurface != nullptr)
			m_ddSurface->Destroy();

		m_ddSurface = nullptr;
		m_port.ClearDirty(PortabilityLayer::QDPortDirtyFlag_Size);
	}

	if (m_ddSurface == nullptr)
		m_ddSurface = displayDriver->CreateSurface(pixMap->m_rect.right - pixMap->m_rect.left, pixMap->m_rect.bottom - pixMap->m_rect.top, pixMap->m_pitch, pixMap->m_pixelFormat, DrawSurface::StaticOnDriverInvalidate, this);

	if (m_port.IsDirty(PortabilityLayer::QDPortDirtyFlag_Contents) && m_ddSurface != nullptr)
	{
		m_ddSurface->UploadEntire(pixMap->m_data, pixMap->m_pitch);
		m_port.ClearDirty(PortabilityLayer::QDPortDirtyFlag_Contents);
	}
}

void DrawSurface::StaticOnDriverInvalidate(void *context)
{
	static_cast<DrawSurface*>(context)->OnDriverInvalidate();
}

void DrawSurface::OnDriverInvalidate()
{
	m_port.SetDirty(PortabilityLayer::QDPortDirtyFlag_Contents);
}
