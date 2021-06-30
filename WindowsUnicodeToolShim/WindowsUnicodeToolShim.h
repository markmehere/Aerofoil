#include <string>
#include <vector>
#include <stdio.h>

FILE *fopen_utf8(const char *path, const char *options);
int fputs_utf8(const char *str, FILE *f);
int mkdir_utf8(const char *path);
void TerminateDirectoryPath(std::string &path);
void ScanDirectoryForExtension(std::vector<std::string>& outPaths, const char *path, const char *ending, bool recursive);

struct DirectoryScanContext;
struct DirectoryScanEntry
{
	const char *m_name;
};

DirectoryScanContext *opendir_utf8(const char *name);
DirectoryScanEntry *readdir_utf8(DirectoryScanContext *dir);
void closedir_utf8(DirectoryScanContext *context);
