#include "GpFontHandler_FreeType2.h"

#include "IOStream.h"
#include "HostFont.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_MODULE_H

#include <stdlib.h>
#include <new>
#include <assert.h>

class GpFont_FreeType2 final : public PortabilityLayer::HostFont
{
public:
	void Destroy() override;

	static GpFont_FreeType2 *Create(FT_Face face);

private:
	explicit GpFont_FreeType2(FT_Face face);
	~GpFont_FreeType2();

	FT_Face m_face;
};


void GpFont_FreeType2::Destroy()
{
	this->~GpFont_FreeType2();
	free(this);
}

GpFont_FreeType2 *GpFont_FreeType2::Create(FT_Face face)
{
	void *storage = malloc(sizeof(GpFont_FreeType2));
	if (!storage)
		return nullptr;

	return new (storage) GpFont_FreeType2(face);
}

GpFont_FreeType2::GpFont_FreeType2(FT_Face face)
	: m_face(face)
{
}

GpFont_FreeType2::~GpFont_FreeType2()
{
	FT_Done_Face(m_face);
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

	FT_Open_Args openArgs;
	memset(&openArgs, 0, sizeof(openArgs));
	openArgs.flags = FT_OPEN_STREAM;
	openArgs.stream = &ftStream;

	FT_Face face;
	FT_Error errorCode = FT_Open_Face(m_library, &openArgs, 0, &face);
	if (errorCode != 0)
		return nullptr;

	GpFont_FreeType2 *font = GpFont_FreeType2::Create(face);
	if (!font)
	{
		FT_Done_Face(face);
		return nullptr;
	}

	return font;
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
