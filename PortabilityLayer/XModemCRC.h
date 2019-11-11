#pragma once

#ifndef __PL_XMODEMCRC_H__
#define __PL_XMODEMCRC_H__

#include "DataTypes.h"

namespace PortabilityLayer
{
	uint16_t XModemCRC(const void *bytes, size_t size, uint16_t initialValue);
}

#endif
