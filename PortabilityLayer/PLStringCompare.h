#pragma once
#ifndef __PL_STRINGCOMPARE_H__
#define __PL_STRINGCOMPARE_H__

#include "PLCore.h"
#include "PLPasStr.h"

Boolean EqualString(const PLPasStr &string1, const PLPasStr &string2, Boolean caseSensitive, Boolean diacriticSensitive);

#endif
