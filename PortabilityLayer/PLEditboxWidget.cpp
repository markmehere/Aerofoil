#include "PLEditboxWidget.h"
#include "PLStandardColors.h"
#include "MemoryManager.h"
#include "FontFamily.h"

#include <algorithm>

namespace PortabilityLayer
{
	EditboxWidget::EditboxWidget(const WidgetBasicState &state)
		: WidgetSpec<EditboxWidget>(state)
		, m_capacity(255)
		, m_length(0)
		, m_chars(nullptr)
		, m_selStartChar(0)
		, m_selEndChar(0)
	{
	}

	EditboxWidget::~EditboxWidget()
	{
		PortabilityLayer::MemoryManager *mm = PortabilityLayer::MemoryManager::GetInstance();

		if (m_chars)
			mm->Release(m_chars);
	}

	bool EditboxWidget::Init(const WidgetBasicState &state)
	{
		PortabilityLayer::MemoryManager *mm = PortabilityLayer::MemoryManager::GetInstance();
		m_capacity = 255;

		m_chars = static_cast<uint8_t*>(mm->Alloc(m_capacity * sizeof(m_chars[0])));
		if (!m_chars)
			return false;

		return true;
	}

	void EditboxWidget::EditboxWidget::DrawControl(DrawSurface *surface)
	{
		const Rect textRect = m_rect;
		const Rect innerRect = textRect.Inset(-2, -2);
		const Rect outerRect = innerRect.Inset(-1, -1);

		surface->SetForeColor(StdColors::Black());
		surface->FillRect(outerRect);
		surface->SetForeColor(StdColors::White());
		surface->FillRect(innerRect);

		surface->SetForeColor(StdColors::Black());
		surface->SetSystemFont(12, PortabilityLayer::FontFamilyFlag_None);
		int32_t ascender = surface->MeasureFontAscender();

		Point basePoint = Point::Create(textRect.left, (textRect.top + textRect.bottom + ascender + 1) / 2);
		surface->DrawStringConstrained(basePoint, this->GetString(), true, m_rect);
	}

	void EditboxWidget::SetString(const PLPasStr &str)
	{
		const size_t len = std::min<size_t>(m_capacity, str.Length());

		m_length = len;
		memcpy(m_chars, str.UChars(), len);
	}

	PLPasStr EditboxWidget::GetString() const
	{
		const uint8_t len = static_cast<uint8_t>(std::min<size_t>(255, m_length));
		return PLPasStr(len, reinterpret_cast<const char*>(m_chars));
	}
}
