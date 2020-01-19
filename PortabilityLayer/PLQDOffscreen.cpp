#include "PLQDOffscreen.h"
#include "MemoryManager.h"
#include "MemReaderStream.h"
#include "ResourceManager.h"
#include "Rect2i.h"
#include "ResTypeID.h"
#include "QDStandardPalette.h"
#include "QDManager.h"
#include "QDGraf.h"
#include "QDPictDecoder.h"
#include "QDPictEmitContext.h"
#include "QDPictEmitScanlineParameters.h"
#include "QDPictHeader.h"
#include "QDPictOpcodes.h"
#include "QDPixMap.h"
#include "Vec2i.h"

#include <string.h>
#include <assert.h>

PLError_t NewGWorld(DrawSurface **gworld, GpPixelFormat_t pixelFormat, const Rect *bounds, CTabHandle colorTable)
{
	return PortabilityLayer::QDManager::GetInstance()->NewGWorld(gworld, pixelFormat, *bounds, colorTable);
}

void DisposeGWorld(DrawSurface *gworld)
{
	return PortabilityLayer::QDManager::GetInstance()->DisposeGWorld(gworld);
}

PixMapHandle GetGWorldPixMap(DrawSurface *gworld)
{
	if (!gworld)
		return nullptr;

	return gworld->m_port.GetPixMap();
}

THandle<BitmapImage> GetPicture(short resID)
{
	return PortabilityLayer::ResourceManager::GetInstance()->GetResource('PICT', resID).StaticCast<BitmapImage>();
}

void OffsetRect(Rect *rect, int right, int down)
{
	rect->left += right;
	rect->right += right;
	rect->top += down;
	rect->bottom += down;
}

DrawSurface *GetGraphicsPort()
{
	PortabilityLayer::QDPort *port = PortabilityLayer::QDManager::GetInstance()->GetPort();

	DrawSurface *grafPtr = reinterpret_cast<DrawSurface *>(port);
	assert(&grafPtr->m_port == port);

	return grafPtr;
}

void SetGraphicsPort(DrawSurface *gw)
{
	PortabilityLayer::QDManager::GetInstance()->SetPort(&gw->m_port);
}
