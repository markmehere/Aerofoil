#include "DeflateCodec.h"

#include "GpIOStream.h"
#include "MemoryManager.h"

#include "zlib.h"

namespace
{
	static voidpf ZlibAllocShim(voidpf opaque, uInt items, uInt size)
	{
		return static_cast<PortabilityLayer::MemoryManager*>(opaque)->Alloc(items * size);
	}

	void ZlibFreeShim(voidpf opaque, voidpf address)
	{
		static_cast<PortabilityLayer::MemoryManager*>(opaque)->Release(address);
	}
}

namespace PortabilityLayer
{
	bool DeflateCodec::DecompressStream(GpIOStream *stream, size_t inSize, void *outBuffer, size_t outSize)
	{
		z_stream zstream;
		zstream.zalloc = ZlibAllocShim;
		zstream.zfree = ZlibFreeShim;
		zstream.opaque = MemoryManager::GetInstance();

		if (inflateInit2(&zstream, -15) != Z_OK)
			return false;

		const size_t bufferSize = 1024;
		uint8_t buffer[1024];

		zstream.avail_out = outSize;
		zstream.next_out = static_cast<Bytef*>(outBuffer);
		zstream.avail_in = 0;
		zstream.next_in = buffer;

		bool failed = false;
		for (;;)
		{
			if (zstream.avail_in == 0)
			{
				const size_t sizeToRead = (bufferSize < inSize) ? bufferSize : inSize;

				if (sizeToRead == 0)
				{
					// Ran out of input
					failed = true;
					break;
				}

				if (stream->Read(buffer, sizeToRead) != sizeToRead)
				{
					failed = true;
					break;
				}

				zstream.avail_in = sizeToRead;
				zstream.next_in = buffer;
			}

			int result = inflate(&zstream, Z_NO_FLUSH);
			if (result == Z_STREAM_END)
			{
				failed = (zstream.avail_out != 0);
				break;
			}

			if (result != Z_OK)
			{
				failed = true;
				break;
			}
		}

		inflateEnd(&zstream);

		return !failed;
	}
}
