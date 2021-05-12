#include <stdio.h>

/*
Copyright 2019 Eric Lasota

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "CFileStream.h"
#include "CombinedTimestamp.h"
#include "DeflateCodec.h"
#include "GpAllocator_C.h"
#include "ScopedPtr.h"
#include "BinHex4.h"
#include "MacBinary2.h"
#include "MacFileMem.h"
#include "ZipFile.h"
#include "WindowsUnicodeToolShim.h"

#include <string>

using namespace PortabilityLayer;

int toolMain(int argc, const char **argv)
{
	if (argc != 4)
	{
		fprintf(stderr, "Usage: hqx2gp <input.hqx> <input.ts> <output>");
		return -1;
	}

	FILE *f = fopen_utf8(argv[1], "rb");

	if (!f)
	{
		fprintf(stderr, "Could not open input file");
		return -1;
	}

	FILE *tsF = fopen_utf8(argv[2], "rb");

	if (!tsF)
	{
		fprintf(stderr, "Could not open timestamp file");
		return -1;
	}

	PortabilityLayer::CombinedTimestamp ts;

	{
		if (fread(&ts, 1, sizeof(ts), tsF) != sizeof(ts))
		{
			fprintf(stderr, "Error reading timestamp file");
			return -1;
		}

	}

	fclose(tsF);

	CFileStream fs(f, true, false, true);

	ScopedPtr<MacFileMem> memFile = BinHex4::LoadHQX(&fs, GpAllocator_C::GetInstance());

	fs.Close();

	std::string fname = argv[3];

	const char* extensions[] = { ".gpf", ".gpr", ".gpd", ".gpc" };

	MacFileProperties mfp = memFile->FileInfo().m_properties;
	mfp.m_createdTimeMacEpoch = mfp.m_modifiedTimeMacEpoch = ts.GetMacEpochTime();

	MacFilePropertiesSerialized sp;
	sp.Serialize(mfp);

	for (int i = 0; i < 4; i++)
	{
		const void *bufferToWrite = nullptr;
		size_t sizeToWrite = 0;

		switch (i)
		{
		case 0:
			bufferToWrite = sp.m_data;
			sizeToWrite = sp.kSize;
			break;
		case 1:
			bufferToWrite = memFile->ResourceFork();
			sizeToWrite = memFile->FileInfo().m_resourceForkSize;
			break;
		case 2:
			bufferToWrite = memFile->DataFork();
			sizeToWrite = memFile->FileInfo().m_dataForkSize;
			break;
		case 3:
			bufferToWrite = memFile->Comment();
			sizeToWrite = memFile->FileInfo().m_commentSize;
			break;
		};

		if (sizeToWrite == 0)
			continue;

		std::string path = fname + extensions[i];

		FILE *outF = fopen_utf8(path.c_str(), "wb");

		if (!outF)
			continue;

		if (i == 0)
		{
			CFileStream stream(outF);
			sp.WriteAsPackage(stream, ts);
			stream.Close();
		}
		else
		{
			fwrite(bufferToWrite, 1, sizeToWrite, outF);
			fclose(outF);
		}
	}

	return 0;
}
