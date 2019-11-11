#pragma once
#ifndef __PL_AE_MANAGER_H__
#define __PL_AE_MANAGER_H__

#include "PLAppleEventsCommonTypes.h"

#include <stdint.h>

namespace PortabilityLayer
{
	class AEManager
	{
	public:
		virtual void Init() = 0;
		virtual void Shutdown() = 0;

		virtual void InstallEventHandler(AEEventClass eventClass, AEEventID eventID, AEEventHandler handler, uint32_t ref, bool isSysHandler) = 0;
		virtual void SetInteractAllowed(AEInteractAllowed interactAllowed) = 0;

		static AEManager *GetInstance();
	};
}

#endif
