#include "FontRenderer.h"

#include "CoreDefs.h"
#include "HostFont.h"
#include "HostFontHandler.h"
#include "HostFontRenderedGlyph.h"
#include "MacRoman.h"
#include "RenderedFont.h"
#include "RenderedGlyphMetrics.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <new>

namespace PortabilityLayer
{
	class RenderedFontImpl final : public RenderedFont
	{
	public:
		bool GetGlyph(unsigned int character, const RenderedGlyphMetrics **outMetricsPtr, const void **outData) const override;

		void Destroy() override;

		void SetCharData(unsigned int charID, const void *data, size_t dataOffset, const RenderedGlyphMetrics &metrics);

		static RenderedFontImpl *Create(size_t glyphDataSize);

	private:
		explicit RenderedFontImpl(void *data);
		~RenderedFontImpl();

		size_t m_dataOffsets[256];
		RenderedGlyphMetrics m_metrics[256];

		void *m_data;
	};

	class FontRendererImpl final : public FontRenderer
	{
	public:
		RenderedFont *RenderFont(HostFont *font, int size, FontHacks fontHacks) override;

		static FontRendererImpl *GetInstance();

	private:
		static FontRendererImpl ms_instance;
	};

	bool RenderedFontImpl::GetGlyph(unsigned int character, const RenderedGlyphMetrics **outMetricsPtr, const void **outData) const
	{
		const size_t dataOffset = m_dataOffsets[character];
		if (!dataOffset)
			return false;

		*outMetricsPtr = m_metrics + character;
		*outData = static_cast<const uint8_t*>(m_data) + dataOffset;

		return true;
	}

	void RenderedFontImpl::Destroy()
	{
		this->~RenderedFontImpl();
		free(this);
	}

	void RenderedFontImpl::SetCharData(unsigned int charID, const void *data, size_t dataOffset, const RenderedGlyphMetrics &metrics)
	{
		m_dataOffsets[charID] = dataOffset;
		m_metrics[charID] = metrics;
		memcpy(static_cast<uint8_t*>(m_data) + dataOffset, data, metrics.m_glyphDataPitch * metrics.m_glyphHeight);
	}

	RenderedFontImpl *RenderedFontImpl::Create(size_t glyphDataSize)
	{
		size_t alignedPrefixSize = sizeof(RenderedFontImpl) + PL_SYSTEM_MEMORY_ALIGNMENT - 1;
		alignedPrefixSize -= alignedPrefixSize % PL_SYSTEM_MEMORY_ALIGNMENT;

		if (SIZE_MAX - alignedPrefixSize < glyphDataSize)
			return nullptr;

		const size_t allocSize = alignedPrefixSize + glyphDataSize;

		void *storage = malloc(allocSize);
		if (!storage)
			return nullptr;

		memset(storage, 0, allocSize);

		return new (storage) RenderedFontImpl(static_cast<uint8_t*>(storage) + alignedPrefixSize);
	}

	RenderedFontImpl::RenderedFontImpl(void *data)
		: m_data(data)
	{
		memset(m_metrics, 0, sizeof(m_metrics));
		memset(m_dataOffsets, 0, sizeof(m_dataOffsets));
	}

	RenderedFontImpl::~RenderedFontImpl()
	{
	}

	RenderedFont *FontRendererImpl::RenderFont(HostFont *font, int size, FontHacks fontHacks)
	{
		const unsigned int numCharacters = 256;

		if (size < 1)
			return nullptr;

		HostFontRenderedGlyph *glyphs[numCharacters];

		for (unsigned int i = 0; i < numCharacters; i++)
			glyphs[i] = nullptr;

		for (unsigned int i = 0; i < numCharacters; i++)
		{
			uint16_t unicodeCodePoint = MacRoman::g_toUnicode[i];
			if (unicodeCodePoint == 0xffff)
				continue;

			glyphs[i] = font->Render(unicodeCodePoint, size);
		}

		size_t glyphDataSize = PL_SYSTEM_MEMORY_ALIGNMENT;	// So we can use 0 to mean no data
		size_t numUsedGlyphs = 0;
		for (unsigned int i = 0; i < numCharacters; i++)
		{
			if (glyphs[i])
			{
				const RenderedGlyphMetrics &metrics = glyphs[i]->GetMetrics();
				glyphDataSize += metrics.m_glyphDataPitch * metrics.m_glyphHeight;
			}
		}

		RenderedFontImpl *rfont = RenderedFontImpl::Create(glyphDataSize);
		if (rfont)
		{
			size_t fillOffset = PL_SYSTEM_MEMORY_ALIGNMENT;

			size_t numUsedGlyphs = 0;
			for (unsigned int i = 0; i < numCharacters; i++)
			{
				if (glyphs[i])
				{
					HostFontRenderedGlyph *glyph = glyphs[i];
					
					RenderedGlyphMetrics metrics = glyph->GetMetrics();
					const void *data = glyph->GetData();

					if (fontHacks == FontHacks_Roboto)
					{
						if (size < 32)
						{
							// 'r' is shifted up 1 pixel
							if (i == 'r' && size < 32)
							{
								metrics.m_bearingY--;
							}

							// ':' doesn't have enough spacing
							if (i == ':')
							{
								metrics.m_bearingX++;
								metrics.m_advanceX++;
							}
						}
					}

					rfont->SetCharData(i, data, fillOffset, metrics);

					fillOffset += metrics.m_glyphDataPitch * metrics.m_glyphHeight;
				}
			}
		}

		for (unsigned int i = 0; i < numCharacters; i++)
		{
			if (glyphs[i])
				glyphs[i]->Destroy();
		}

		return rfont;
	}

	FontRendererImpl *FontRendererImpl::GetInstance()
	{
		return &ms_instance;
	}

	FontRendererImpl FontRendererImpl::ms_instance;

	FontRenderer *FontRenderer::GetInstance()
	{
		return FontRendererImpl::GetInstance();
	}
}
