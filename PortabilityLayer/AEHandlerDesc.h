#pragma once
#ifndef __PL_AE_HANDLER_DESC_H__
#define __PL_AE_HANDLER_DESC_H__

#include "PLAppleEventsCommonTypes.h"

#include <stdint.h>

namespace PortabilityLayer
{
	struct AEHandlerDesc
	{
		AEEventClass m_eventClass;
		AEEventID m_eventID;
		AEEventHandler m_handler;
		uint32_t m_ref;
	};
}

#endif
