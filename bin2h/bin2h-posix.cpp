#include <dirent.h>
#include <sys/stat.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

int main(int argc, const char **argv)
{
	if (argc != 3)
	{
		fprintf(stderr, "Usage: bin2h <dir> <output>\n");
		return -1;
	}

	std::string dir = argv[1];
	DIR *dirp = opendir(dir.c_str());
	if (!dirp)
	{
		perror("Failed to open directory");
		return -1;
	}

	FILE *outF = fopen(argv[2], "wb");
	if (!outF)
	{
		perror("Failed to open output file");
		closedir(dirp);
		return -1;
	}

	std::vector<std::string> fileNames;
	int numFiles = 0;

	struct dirent *entry;
	while ((entry = readdir(dirp)) != nullptr)
	{
		if (entry->d_type == DT_REG) // Regular file
		{
			fileNames.push_back(entry->d_name);

			std::string filePath = dir + "/" + entry->d_name;

			FILE *inF = fopen(filePath.c_str(), "rb");
			if (!inF)
			{
				perror("Failed to open input file");
				fclose(outF);
				closedir(dirp);
				return -1;
			}

			fprintf(outF, "const unsigned char g_fileContents%i[] = {\n", numFiles);

			fseek(inF, 0, SEEK_END);
			long fileSize = ftell(inF);
			fseek(inF, 0, SEEK_SET);

			for (long i = 0; i < fileSize; i++)
			{
				if (i % 20 == 0)
					fputs("\n\t\t", outF);

				int c = fgetc(inF);
				fprintf(outF, "% 4i,", (c & 0xff));
			}
			fprintf(outF, "\n};\n\n");

			numFiles++;

			fclose(inF);
		}
	}

	closedir(dirp);

	fprintf(outF, "FileCatalogEntry g_catalogEntries[] = {\n");
	for (int i = 0; i < numFiles; i++)
		fprintf(outF, "\t{ \"%s\", g_fileContents%i, sizeof(g_fileContents%i) },\n", fileNames[i].c_str(), i, i);
	fprintf(outF, "};\n");
	fprintf(outF, "FileCatalog g_catalog = { g_catalogEntries, sizeof(g_catalogEntries) / sizeof(g_catalogEntries[0]), sizeof(g_catalogEntries), sizeof(g_catalogEntries[0]) };\n");
	fprintf(outF, "const FileCatalog &GetCatalog() { return g_catalog; }\n");

	fclose(outF);

	return 0;
}
