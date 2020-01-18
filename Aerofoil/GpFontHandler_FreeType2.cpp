#include "GpFontHandler_FreeType2.h"

#include "CoreDefs.h"
#include "IOStream.h"
#include "HostFont.h"
#include "HostFontRenderedGlyph.h"
#include "RenderedGlyphMetrics.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_MODULE_H

#include <stdint.h>
#include <stdlib.h>
#include <new>
#include <assert.h>


class GpFontRenderedGlyph_FreeType2 final : public PortabilityLayer::HostFontRenderedGlyph
{
public:
	const PortabilityLayer::RenderedGlyphMetrics &GetMetrics() const override;
	const void *GetData() const override;
	void Destroy() override;

	static GpFontRenderedGlyph_FreeType2 *Create(size_t dataSize, const PortabilityLayer::RenderedGlyphMetrics &metrics);

	void *GetMutableData();

private:
	GpFontRenderedGlyph_FreeType2(void *data, const PortabilityLayer::RenderedGlyphMetrics &metrics);
	~GpFontRenderedGlyph_FreeType2();

	void *m_data;
	PortabilityLayer::RenderedGlyphMetrics m_metrics;
};

class GpFont_FreeType2 final : public PortabilityLayer::HostFont
{
public:
	void Destroy() override;
	GpFontRenderedGlyph_FreeType2 *Render(uint32_t unicodeCodePoint, unsigned int size, bool aa) override;
	bool GetLineSpacing(unsigned int size, int32_t &outSpacing) override;

	static GpFont_FreeType2 *Create(const FT_StreamRec_ &streamRec, PortabilityLayer::IOStream *stream);

	bool FTLoad(const FT_Library &library);

private:
	explicit GpFont_FreeType2(const FT_StreamRec_ &streamRec, PortabilityLayer::IOStream *stream);
	~GpFont_FreeType2();

	FT_StreamRec_ m_ftStream;
	FT_Face m_face;
	PortabilityLayer::IOStream *m_stream;
	unsigned int m_currentSize;
};

const PortabilityLayer::RenderedGlyphMetrics &GpFontRenderedGlyph_FreeType2::GetMetrics() const
{
	return m_metrics;
}

const void *GpFontRenderedGlyph_FreeType2::GetData() const
{
	return m_data;
}

void GpFontRenderedGlyph_FreeType2::Destroy()
{
	this->~GpFontRenderedGlyph_FreeType2();
	free(this);
}

GpFontRenderedGlyph_FreeType2 *GpFontRenderedGlyph_FreeType2::Create(size_t dataSize, const PortabilityLayer::RenderedGlyphMetrics &metrics)
{
	size_t alignedPrefixSize = (sizeof(GpFontRenderedGlyph_FreeType2) + GP_SYSTEM_MEMORY_ALIGNMENT - 1);
	alignedPrefixSize -= alignedPrefixSize % GP_SYSTEM_MEMORY_ALIGNMENT;

	void *storage = malloc(alignedPrefixSize + dataSize);
	if (!storage)
		return nullptr;

	return new (storage) GpFontRenderedGlyph_FreeType2(static_cast<uint8_t*>(storage) + alignedPrefixSize, metrics);
}

void *GpFontRenderedGlyph_FreeType2::GetMutableData()
{
	return m_data;
}


GpFontRenderedGlyph_FreeType2::GpFontRenderedGlyph_FreeType2(void *data, const PortabilityLayer::RenderedGlyphMetrics &metrics)
	: m_metrics(metrics)
	, m_data(data)
{
}

GpFontRenderedGlyph_FreeType2::~GpFontRenderedGlyph_FreeType2()
{
}

void GpFont_FreeType2::Destroy()
{
	this->~GpFont_FreeType2();
	free(this);
}

GpFontRenderedGlyph_FreeType2 *GpFont_FreeType2::Render(uint32_t unicodeCodePoint, unsigned int size, bool aa)
{
	if (m_currentSize != size)
	{
		if (FT_Set_Pixel_Sizes(m_face, 0, size) != 0)
			return nullptr;

		m_currentSize = size;
	}

	FT_UInt glyphIndex = FT_Get_Char_Index(m_face, unicodeCodePoint);
	if (!glyphIndex)
		return nullptr;

	FT_Int32 loadFlags = 0;
	FT_Render_Mode renderMode = FT_RENDER_MODE_NORMAL;

	if (aa)
	{
		renderMode = FT_RENDER_MODE_NORMAL;
		loadFlags = FT_LOAD_TARGET_NORMAL;
	}
	else
	{
		loadFlags = FT_LOAD_MONOCHROME | FT_LOAD_TARGET_MONO;
		renderMode = FT_RENDER_MODE_MONO;
	}

	if (FT_Load_Glyph(m_face, glyphIndex, loadFlags) != 0)
		return nullptr;

	if (FT_Render_Glyph(m_face->glyph, renderMode) != 0)
		return nullptr;

	const FT_GlyphSlot glyph = m_face->glyph;

	if (aa == false && glyph->bitmap.pixel_mode != FT_PIXEL_MODE_MONO)
		return nullptr;

	if (aa == true)
	{
		if (glyph->bitmap.pixel_mode != FT_PIXEL_MODE_GRAY)
			return nullptr;

		if (glyph->bitmap.num_grays < 2)
			return nullptr;	// This should never happen
	}

	PortabilityLayer::RenderedGlyphMetrics metrics;
	memset(&metrics, 0, sizeof(metrics));

	metrics.m_bearingX = glyph->metrics.horiBearingX / 64;
	metrics.m_bearingY = glyph->metrics.horiBearingY / 64;
	metrics.m_glyphWidth = glyph->bitmap.width;
	metrics.m_glyphHeight = glyph->bitmap.rows;
	metrics.m_advanceX = glyph->metrics.horiAdvance / 64;

	const size_t numRowsRequired = glyph->bitmap.rows;
	size_t pitchRequired = 0;

	if (aa)
		pitchRequired = (glyph->bitmap.width + 1) / 2;
	else
		pitchRequired = (glyph->bitmap.width + 7) / 8;

	pitchRequired = pitchRequired + (GP_SYSTEM_MEMORY_ALIGNMENT - 1);
	pitchRequired -= pitchRequired % GP_SYSTEM_MEMORY_ALIGNMENT;

	const size_t glyphDataSize = numRowsRequired * pitchRequired;

	metrics.m_glyphDataPitch = pitchRequired;

	GpFontRenderedGlyph_FreeType2 *renderedGlyph = GpFontRenderedGlyph_FreeType2::Create(glyphDataSize, metrics);
	if (!renderedGlyph)
		return nullptr;

	uint8_t *fillData = static_cast<uint8_t*>(renderedGlyph->GetMutableData());

	unsigned int bmWidth = glyph->bitmap.width;
	unsigned int bmHeight = glyph->bitmap.rows;
	unsigned int bmPitch = glyph->bitmap.pitch;
	const uint8_t *bmBytes = glyph->bitmap.buffer;
	const uint16_t numGrays = glyph->bitmap.num_grays;

	size_t fillOffset = 0;
	for (unsigned int row = 0; row < bmHeight; row++)
	{
		const uint8_t *bmReadStart = bmBytes + bmPitch * row;
		uint8_t *bmWriteStart = fillData + pitchRequired * row;

		if (aa)
		{
			for (size_t i = 0; i < pitchRequired; i++)
				bmWriteStart[i] = 0;

			for (size_t i = 0; i < bmWidth; i++)
			{
				const uint8_t b = bmReadStart[i];
				const uint8_t normalizedGray = (b * 30 + (numGrays - 1)) / ((numGrays - 1) * 2);

				if (i & 1)
					bmWriteStart[i / 2] |= (normalizedGray << 4);
				else
					bmWriteStart[i / 2] = normalizedGray;
			}
		}
		else
		{
			const size_t copyableBytesPerRow = (glyph->bitmap.width + 7) / 8;

			for (size_t i = 0; i < copyableBytesPerRow; i++)
			{
				const uint8_t b = bmReadStart[i];

				uint8_t fillByte = 0;
				for (int bit = 0; bit < 8; bit++)
					fillByte |= ((b >> (7 - bit)) & 1) << bit;

				bmWriteStart[i] = fillByte;
			}
		}
	}

	return renderedGlyph;
}

bool GpFont_FreeType2::GetLineSpacing(unsigned int size, int32_t &outSpacing)
{
	if (m_currentSize != size)
	{
		if (FT_Set_Pixel_Sizes(m_face, 0, size) != 0)
			return false;

		m_currentSize = size;
	}

	outSpacing = m_face->size->metrics.height / 64;
	return true;
}


GpFont_FreeType2 *GpFont_FreeType2::Create(const FT_StreamRec_ &streamRec, PortabilityLayer::IOStream *stream)
{
	void *storage = malloc(sizeof(GpFont_FreeType2));
	if (!storage)
		return nullptr;

	return new (storage) GpFont_FreeType2(streamRec, stream);
}

bool GpFont_FreeType2::FTLoad(const FT_Library &library)
{
	FT_Open_Args openArgs;
	memset(&openArgs, 0, sizeof(openArgs));
	openArgs.flags = FT_OPEN_STREAM;
	openArgs.stream = &m_ftStream;

	FT_Error errorCode = FT_Open_Face(library, &openArgs, 0, &m_face);
	if (errorCode != 0)
		return false;

	return true;
}

GpFont_FreeType2::GpFont_FreeType2(const FT_StreamRec_ &streamRec, PortabilityLayer::IOStream *stream)
	: m_face(nullptr)
	, m_ftStream(streamRec)
	, m_stream(stream)
	, m_currentSize(0)
{
	assert(stream);
}

GpFont_FreeType2::~GpFont_FreeType2()
{
	if (m_face)
		FT_Done_Face(m_face);

	m_stream->Close();
}

GpFontHandler_FreeType2 *GpFontHandler_FreeType2::Create()
{
	void *storage = malloc(sizeof(GpFontHandler_FreeType2));
	if (!storage)
		return nullptr;

	GpFontHandler_FreeType2 *fh = new (storage) GpFontHandler_FreeType2();
	if (!fh->Init())
	{
		fh->Shutdown();
		return nullptr;
	}

	return fh;
}

PortabilityLayer::HostFont *GpFontHandler_FreeType2::LoadFont(PortabilityLayer::IOStream *stream)
{
	FT_StreamRec_ ftStream;
	memset(&ftStream, 0, sizeof(ftStream));
	ftStream.size = 0x7fffffff;
	ftStream.pos = 0;
	ftStream.descriptor.pointer = stream;
	ftStream.read = FTStreamIo;
	ftStream.close = FTStreamClose;

	GpFont_FreeType2 *font = GpFont_FreeType2::Create(ftStream, stream);
	if (!font)
	{
		stream->Close();
		return nullptr;
	}

	if (!font->FTLoad(m_library))
	{
		font->Destroy();
		return nullptr;
	}

	return font;
}

bool GpFontHandler_FreeType2::KeepStreamOpen() const
{
	return true;
}

void GpFontHandler_FreeType2::Shutdown()
{
	this->~GpFontHandler_FreeType2();
	free(this);
}

GpFontHandler_FreeType2::GpFontHandler_FreeType2()
	: m_ftIsInitialized(false)
	, m_library(nullptr)
	, m_currentSize(0)
{
}

GpFontHandler_FreeType2::~GpFontHandler_FreeType2()
{
	if (m_ftIsInitialized)
		FT_Done_Library(m_library);
}


void *GpFontHandler_FreeType2::FTAllocThunk(FT_Memory memory, long size)
{
	return static_cast<GpFontHandler_FreeType2*>(memory->user)->FTAlloc(size);
}

void GpFontHandler_FreeType2::FTFreeThunk(FT_Memory memory, void* block)
{
	static_cast<GpFontHandler_FreeType2*>(memory->user)->FTFree(block);
}

void *GpFontHandler_FreeType2::FTReallocThunk(FT_Memory memory, long curSize, long newSize, void *block)
{
	return static_cast<GpFontHandler_FreeType2*>(memory->user)->FTRealloc(curSize, newSize, block);
}


unsigned long GpFontHandler_FreeType2::FTStreamIo(FT_Stream stream, unsigned long offset, unsigned char* buffer, unsigned long count)
{
	PortabilityLayer::IOStream *ioStream = static_cast<PortabilityLayer::IOStream*>(stream->descriptor.pointer);

	if (count == 0)
	{
		if (!ioStream->SeekStart(static_cast<PortabilityLayer::UFilePos_t>(offset)))
			return 1;

		return 0;
	}

	const size_t bytesRead = ioStream->Read(buffer, count);
	return static_cast<unsigned long>(bytesRead);
}

void GpFontHandler_FreeType2::FTStreamClose(FT_Stream stream)
{
	(void)stream;
}

void *GpFontHandler_FreeType2::FTAlloc(long size)
{
	return malloc(static_cast<size_t>(size));
}

void GpFontHandler_FreeType2::FTFree(void* block)
{
	free(block);
}

void *GpFontHandler_FreeType2::FTRealloc(long curSize, long newSize, void *block)
{
	(void)curSize;
	return realloc(block, static_cast<size_t>(newSize));
}

bool GpFontHandler_FreeType2::Init()
{
	m_mem.user = this;
	m_mem.alloc = FTAllocThunk;
	m_mem.free = FTFreeThunk;
	m_mem.realloc = FTReallocThunk;

	if (FT_New_Library(&m_mem, &m_library) != 0)
		return false;

	m_ftIsInitialized = true;

	FT_Add_Default_Modules(m_library);

	return true;
}
