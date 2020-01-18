#include "FontRenderer.h"

#include "CoreDefs.h"
#include "HostFont.h"
#include "HostFontHandler.h"
#include "HostFontRenderedGlyph.h"
#include "MacRomanConversion.h"
#include "PLPasStr.h"
#include "RenderedFont.h"
#include "RenderedFontMetrics.h"
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
		bool GetGlyph(unsigned int character, const RenderedGlyphMetrics *&outMetricsPtr, const void *&outData) const override;
		const RenderedFontMetrics &GetMetrics() const override;
		size_t MeasureString(const uint8_t *chars, size_t len) const override;
		bool IsAntiAliased() const override;

		void Destroy() override;

		void SetCharData(unsigned int charID, const void *data, size_t dataOffset, const RenderedGlyphMetrics &metrics);
		void SetFontMetrics(const RenderedFontMetrics &metrics);

		static RenderedFontImpl *Create(size_t glyphDataSize, bool aa);

	private:
		RenderedFontImpl(void *data, bool aa);
		~RenderedFontImpl();

		size_t m_dataOffsets[256];
		RenderedGlyphMetrics m_glyphMetrics[256];

		RenderedFontMetrics m_fontMetrics;
		bool m_isAntiAliased;

		void *m_data;
	};

	class FontRendererImpl final : public FontRenderer
	{
	public:
		RenderedFont *RenderFont(HostFont *font, int size, bool aa, FontHacks fontHacks) override;

		static FontRendererImpl *GetInstance();

	private:
		static FontRendererImpl ms_instance;
	};

	bool RenderedFontImpl::GetGlyph(unsigned int character, const RenderedGlyphMetrics *&outMetricsPtr, const void *&outData) const
	{
		const size_t dataOffset = m_dataOffsets[character];
		if (!dataOffset)
			return false;

		outMetricsPtr = m_glyphMetrics + character;
		outData = static_cast<const uint8_t*>(m_data) + dataOffset;

		return true;
	}

	const RenderedFontMetrics &RenderedFontImpl::GetMetrics() const
	{
		return m_fontMetrics;
	}

	size_t RenderedFontImpl::MeasureString(const uint8_t *chars, size_t len) const
	{
		size_t measure = 0;

		for (size_t i = 0; i < len; i++)
		{
			const RenderedGlyphMetrics &metrics = m_glyphMetrics[chars[i]];
			measure += metrics.m_advanceX;
		}

		return measure;
	}

	bool RenderedFontImpl::IsAntiAliased() const
	{
		return m_isAntiAliased;
	}

	void RenderedFontImpl::Destroy()
	{
		this->~RenderedFontImpl();
		free(this);
	}

	void RenderedFontImpl::SetCharData(unsigned int charID, const void *data, size_t dataOffset, const RenderedGlyphMetrics &metrics)
	{
		m_dataOffsets[charID] = dataOffset;
		m_glyphMetrics[charID] = metrics;
		memcpy(static_cast<uint8_t*>(m_data) + dataOffset, data, metrics.m_glyphDataPitch * metrics.m_glyphHeight);
	}

	void RenderedFontImpl::SetFontMetrics(const RenderedFontMetrics &metrics)
	{
		m_fontMetrics = metrics;
	}

	RenderedFontImpl *RenderedFontImpl::Create(size_t glyphDataSize, bool aa)
	{
		size_t alignedPrefixSize = sizeof(RenderedFontImpl) + GP_SYSTEM_MEMORY_ALIGNMENT - 1;
		alignedPrefixSize -= alignedPrefixSize % GP_SYSTEM_MEMORY_ALIGNMENT;

		if (SIZE_MAX - alignedPrefixSize < glyphDataSize)
			return nullptr;

		const size_t allocSize = alignedPrefixSize + glyphDataSize;

		void *storage = malloc(allocSize);
		if (!storage)
			return nullptr;

		memset(storage, 0, allocSize);

		return new (storage) RenderedFontImpl(static_cast<uint8_t*>(storage) + alignedPrefixSize, aa);
	}

	RenderedFontImpl::RenderedFontImpl(void *data, bool aa)
		: m_data(data)
		, m_isAntiAliased(aa)
	{
		memset(m_glyphMetrics, 0, sizeof(m_glyphMetrics));
		memset(&m_fontMetrics, 0, sizeof(m_fontMetrics));
		memset(m_dataOffsets, 0, sizeof(m_dataOffsets));
	}

	RenderedFontImpl::~RenderedFontImpl()
	{
	}

	RenderedFont *FontRendererImpl::RenderFont(HostFont *font, int size, bool aa, FontHacks fontHacks)
	{
		const unsigned int numCharacters = 256;

		if (size < 1)
			return nullptr;

		int32_t lineSpacing;
		if (!font->GetLineSpacing(size, lineSpacing))
			return nullptr;

		HostFontRenderedGlyph *glyphs[numCharacters];

		for (unsigned int i = 0; i < numCharacters; i++)
			glyphs[i] = nullptr;

		for (unsigned int i = 0; i < numCharacters; i++)
		{
			uint16_t unicodeCodePoint = MacRoman::ToUnicode(i);
			if (unicodeCodePoint == 0xffff)
				continue;

			glyphs[i] = font->Render(unicodeCodePoint, size, aa);
		}

		size_t glyphDataSize = GP_SYSTEM_MEMORY_ALIGNMENT;	// So we can use 0 to mean no data
		size_t numUsedGlyphs = 0;
		for (unsigned int i = 0; i < numCharacters; i++)
		{
			if (glyphs[i])
			{
				const RenderedGlyphMetrics &metrics = glyphs[i]->GetMetrics();
				glyphDataSize += metrics.m_glyphDataPitch * metrics.m_glyphHeight;
			}
		}

		RenderedFontImpl *rfont = RenderedFontImpl::Create(glyphDataSize, aa);
		if (rfont)
		{
			size_t fillOffset = GP_SYSTEM_MEMORY_ALIGNMENT;

			size_t numUsedGlyphs = 0;
			for (unsigned int i = 0; i < numCharacters; i++)
			{
				if (glyphs[i])
				{
					HostFontRenderedGlyph *glyph = glyphs[i];
					
					RenderedGlyphMetrics metrics = glyph->GetMetrics();
					const void *data = glyph->GetData();

					if (fontHacks == FontHacks_Roboto && !aa)
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

		// Compute metrics
		RenderedFontMetrics fontMetrics;
		fontMetrics.m_linegap = lineSpacing;
		fontMetrics.m_ascent = 0;
		fontMetrics.m_descent = 0;

		bool measuredAnyGlyphs = false;
		for (char capChar = 'A'; capChar <= 'Z'; capChar++)
		{
			const RenderedGlyphMetrics *glyphMetrics;
			const void *glyphData;
			if (rfont->GetGlyph(static_cast<unsigned int>(capChar), glyphMetrics, glyphData) && glyphMetrics != nullptr)
			{
				const int32_t ascent = glyphMetrics->m_bearingY;
				const int32_t descent = static_cast<int32_t>(glyphMetrics->m_glyphHeight) - ascent;

				if (!measuredAnyGlyphs)
				{
					fontMetrics.m_ascent = ascent;
					fontMetrics.m_descent = descent;
					measuredAnyGlyphs = true;
				}
				else
				{
					if (ascent > fontMetrics.m_ascent)
						fontMetrics.m_ascent = ascent;

					if (descent > fontMetrics.m_descent)
						fontMetrics.m_descent = descent;
				}
			}
		}

		rfont->SetFontMetrics(fontMetrics);

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
