#include "FontManager.h"
#include "FontFamily.h"
#include "FontRenderer.h"

#include "HostFileSystem.h"
#include "HostFont.h"
#include "HostFontHandler.h"
#include "IOStream.h"
#include "RenderedFont.h"

#include <string.h>
#include <stdlib.h>

void PL_NotYetImplemented();

namespace PortabilityLayer
{
	class FontManagerImpl final : public FontManager
	{
	public:
		void Init() override;
		void Shutdown() override;

		FontFamily *GetSystemFont(int textSize, int variationFlags) const override;
		FontFamily *GetApplicationFont(int textSize, int variationFlags) const override;

		RenderedFont *GetRenderedFont(HostFont *font, int size, bool aa, FontHacks fontHacks) override;
		RenderedFont *GetRenderedFontFromFamily(FontFamily *font, int size, bool aa, int flags) override;

		static FontManagerImpl *GetInstance();

	private:
		static const unsigned int kNumCachedRenderedFonts = 32;

		struct CachedRenderedFont
		{
			RenderedFont *m_rfont;
			const HostFont *m_font;
			int m_size;
			uint32_t m_lastUsage;
			bool m_aa;
		};

		FontManagerImpl();

		void ResetUsageCounter();
		static int CRFSortPredicate(const void *a, const void *b);

		FontFamily *m_systemFont;
		FontFamily *m_applicationFont;
		uint32_t m_usageCounter;

		CachedRenderedFont m_cachedRenderedFonts[kNumCachedRenderedFonts];

		static FontManagerImpl ms_instance;
	};

	void FontManagerImpl::Init()
	{
		m_systemFont = FontFamily::Create();
		m_applicationFont = FontFamily::Create();

		if (m_systemFont)
			m_systemFont->AddFont(FontFamilyFlag_None, "Fonts/OpenSans/OpenSans-ExtraBold.ttf", FontHacks_None);

		if (m_applicationFont)
		{
			m_applicationFont->AddFont(FontFamilyFlag_None, "Fonts/OpenSans/OpenSans-SemiBold.ttf", FontHacks_None);
			m_applicationFont->AddFont(FontFamilyFlag_Bold, "Fonts/OpenSans/OpenSans-Bold.ttf", FontHacks_None);
		}

		memset(m_cachedRenderedFonts, 0, sizeof(m_cachedRenderedFonts));
	}

	void FontManagerImpl::Shutdown()
	{
		if (m_systemFont)
			m_systemFont->Destroy();

		if (m_applicationFont)
			m_applicationFont->Destroy();

		HostFontHandler *hfh = HostFontHandler::GetInstance();

		if (m_systemFont)
		{
			m_systemFont->Destroy();
			m_systemFont = nullptr;
		}

		for (int i = 0; i < sizeof(m_cachedRenderedFonts) / sizeof(m_cachedRenderedFonts[0]); i++)
		{
			CachedRenderedFont *crf = m_cachedRenderedFonts + i;
			if (crf->m_rfont)
				crf->m_rfont->Destroy();
		}
	}

	FontFamily *FontManagerImpl::GetSystemFont(int textSize, int variationFlags) const
	{
		(void)textSize;
		return m_systemFont;
	}

	FontFamily *FontManagerImpl::GetApplicationFont(int textSize, int variationFlags) const
	{
		if (textSize < 11 && (variationFlags & FontFamilyFlag_Bold) != 0)
			return m_systemFont;	// Use heavier font below 11pt

		return m_applicationFont;
	}

	RenderedFont *FontManagerImpl::GetRenderedFont(HostFont *font, int size, bool aa, FontHacks fontHacks)
	{
		CachedRenderedFont *newCacheSlot = &m_cachedRenderedFonts[0];

		for (int i = 0; i < kNumCachedRenderedFonts; i++)
		{
			CachedRenderedFont &crf = m_cachedRenderedFonts[i];
			if (crf.m_rfont == nullptr)
			{
				newCacheSlot = &crf;
				break;
			}

			if (crf.m_font == font && crf.m_size == size && crf.m_aa == aa)
			{
				crf.m_lastUsage = m_usageCounter;
				RenderedFont *rf = crf.m_rfont;
				if (m_usageCounter == UINT32_MAX)
					ResetUsageCounter();
				else
					m_usageCounter++;

				return rf;
			}

			if (newCacheSlot->m_rfont != nullptr && crf.m_lastUsage < newCacheSlot->m_lastUsage)
				newCacheSlot = &crf;
		}

		RenderedFont *rfont = FontRenderer::GetInstance()->RenderFont(font, size, aa, fontHacks);
		if (!rfont)
			return nullptr;

		if (newCacheSlot->m_rfont)
			newCacheSlot->m_rfont->Destroy();

		newCacheSlot->m_font = font;
		newCacheSlot->m_lastUsage = m_usageCounter;
		newCacheSlot->m_size = size;
		newCacheSlot->m_rfont = rfont;
		newCacheSlot->m_aa = aa;

		if (m_usageCounter == UINT32_MAX)
			ResetUsageCounter();
		else
			m_usageCounter++;

		return rfont;
	}

	RenderedFont *FontManagerImpl::GetRenderedFontFromFamily(FontFamily *fontFamily, int size, bool aa, int flags)
	{
		const int variation = fontFamily->GetVariationForFlags(flags);

		PortabilityLayer::HostFont *hostFont = fontFamily->GetFontForVariation(variation);
		if (!hostFont)
			return nullptr;

		return PortabilityLayer::FontManager::GetInstance()->GetRenderedFont(hostFont, size, aa, fontFamily->GetHacksForVariation(variation));
	}

	FontManagerImpl *FontManagerImpl::GetInstance()
	{
		return &ms_instance;
	}

	FontManagerImpl::FontManagerImpl()
		: m_systemFont(nullptr)
	{
	}

	void FontManagerImpl::ResetUsageCounter()
	{
		// Resets the usage counter if it would overflow by sorting by ascending last usage and then resetting all counts to simple +1 increments
		qsort(m_cachedRenderedFonts, kNumCachedRenderedFonts, sizeof(CachedRenderedFont), FontManagerImpl::CRFSortPredicate);

		m_usageCounter = 0;

		for (unsigned int i = 0; i < kNumCachedRenderedFonts; i++)
		{
			CachedRenderedFont &crf = m_cachedRenderedFonts[i];

			if (!crf.m_rfont)
				break;

			crf.m_lastUsage = m_usageCounter++;
		}
	}

	int FontManagerImpl::CRFSortPredicate(const void *a, const void *b)
	{
		const CachedRenderedFont *crfA = static_cast<const CachedRenderedFont *>(a);
		const CachedRenderedFont *crfB = static_cast<const CachedRenderedFont *>(b);

		if (crfA->m_rfont == nullptr && crfB->m_rfont == nullptr)
			return 0;

		if (crfA->m_rfont == nullptr && crfB->m_rfont != nullptr)
			return 1;

		if (crfA->m_rfont != nullptr && crfB->m_rfont == nullptr)
			return -1;

		if (crfA->m_lastUsage < crfB->m_lastUsage)
			return -1;

		if (crfA->m_lastUsage > crfB->m_lastUsage)
			return 1;

		return 0;
	}

	FontManagerImpl FontManagerImpl::ms_instance;

	FontManager *FontManager::GetInstance()
	{
		return FontManagerImpl::GetInstance();
	}
}
