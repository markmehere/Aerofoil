#include "BMPFormat.h"
#include "BitmapImage.h"

Rect BitmapImage::GetRect() const
{
	const PortabilityLayer::BitmapInfoHeader *infoHeader = reinterpret_cast<const PortabilityLayer::BitmapInfoHeader*>(reinterpret_cast<const uint8_t*>(this) + sizeof(PortabilityLayer::BitmapFileHeader));

	const uint32_t width = infoHeader->m_width;
	const uint32_t height = infoHeader->m_height;

	return Rect::Create(0, 0, static_cast<int16_t>(height), static_cast<int16_t>(width));
}
