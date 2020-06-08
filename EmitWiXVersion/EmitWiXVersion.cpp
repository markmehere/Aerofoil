#include "GpBuildVersion.h"
#include "WindowsUnicodeToolShim.h"


int toolMain(int argc, const char **argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "Usage: EmitWiXVersion <output>\n");
		return -1;
	}

	FILE *f = fopen_utf8(argv[1], "wb");
	if (!f)
	{
		fprintf(stderr, "EmitWiXVersion: Error opening output file\n");
		return -1;
	}

	fprintf(f, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
	fprintf(f, "<Include>\n");
	fprintf(f, "  <?define AerofoilReleaseVersion=\"%i.%i.%i.0\" ?>\n", GP_BUILD_VERSION_MAJOR, GP_BUILD_VERSION_MINOR, GP_BUILD_VERSION_UPDATE);
	fprintf(f, "</Include>\n");
	fclose(f);

	return 0;
}
