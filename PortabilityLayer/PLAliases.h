#pragma once
#ifndef __PL_ALIASES_H__
#define __PL_ALIASES_H__

#include "PLCore.h"

OSErr ResolveAliasFile(FSSpecPtr fsSpec, Boolean recursive, Boolean *outIsFolder, Boolean *outWasAliased);

#endif
