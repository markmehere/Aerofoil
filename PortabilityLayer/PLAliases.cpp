#include "PLAliases.h"

OSErr ResolveAliasFile(FSSpecPtr fsSpec, Boolean recursive, Boolean *outIsFolder, Boolean *outWasAliased)
{
	*outIsFolder = PL_FALSE;
	*outWasAliased = PL_FALSE;
	return noErr;
}
