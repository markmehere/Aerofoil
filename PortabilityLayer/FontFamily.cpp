#include "FontFamily.h"
#include "IOStream.h"
#include "HostFileSystem.h"
#include "HostFontHandler.h"
#include "HostFont.h"

#include <stdlib.h>
#include <new>

namespace PortabilityLayer
{
	void FontFamily::AddFont(int flags, const char *path, FontHacks fontHacks)
	{
		PortabilityLayer::IOStream *sysFontStream = PortabilityLayer::HostFileSystem::GetInstance()->OpenFile(PortabilityLayer::EVirtualDirectory_Fonts, path, false, false);
		if (!sysFontStream)
			return;

		PortabilityLayer::HostFontHandler *fontHandler = PortabilityLayer::HostFontHandler::GetInstance();

		PortabilityLayer::HostFont *font = fontHandler->LoadFont(sysFontStream);

		if (!fontHandler->KeepStreamOpen())
			sysFontStream->Close();

		if (!font)
			return;

		m_fonts[flags] = font;
		m_hacks[flags] = fontHacks;

		if (m_fonts[0] == nullptr)
			m_defaultVariation = flags;
	}

	void FontFamily::SetDefaultVariation(int defaultVariation)
	{
		if (m_fonts[defaultVariation])
			m_defaultVariation = defaultVariation;
	}

	int FontFamily::GetVariationForFlags(int variation) const
	{
		if (m_fonts[variation])
			return variation;

		if (m_fonts[0])
			return 0;

		return m_defaultVariation;
	}

	PortabilityLayer::HostFont *FontFamily::GetFontForVariation(int variation) const
	{
		return m_fonts[variation];
	}

	PortabilityLayer::FontHacks FontFamily::GetHacksForVariation(int variation) const
	{
		return m_hacks[variation];
	}

	FontFamily *FontFamily::Create()
	{
		void *storage = malloc(sizeof(FontFamily));
		if (!storage)
			return nullptr;

		return new (storage) FontFamily();
	}

	void FontFamily::Destroy()
	{
		this->~FontFamily();
		free(this);
	}

	FontFamily::FontFamily()
		: m_defaultVariation(0)
	{
		for (unsigned int i = 0; i < kNumVariations; i++)
		{
			m_fonts[i] = nullptr;
			m_hacks[i] = FontHacks_None;
		}
	}

	FontFamily::~FontFamily()
	{
		for (unsigned int i = 0; i < kNumVariations; i++)
		{
			if (PortabilityLayer::HostFont *font = m_fonts[i])
				font->Destroy();
		}
	}
}
