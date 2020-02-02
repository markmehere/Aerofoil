#pragma once

#include "GpVOSEvent.h"

struct TimeTaggedVOSEvent
{
	GpVOSEvent m_vosEvent;
	uint32_t m_timestamp;

	static TimeTaggedVOSEvent Create(const GpVOSEvent &vosEvent, uint32_t timestamp);

	// Helpers for common cases
	bool IsKeyDownEvent() const;	// Only returns true for untranslated keydown events
	bool IsLMouseDownEvent() const;
	bool IsLMouseUpEvent() const;
};

inline TimeTaggedVOSEvent TimeTaggedVOSEvent::Create(const GpVOSEvent &vosEvent, uint32_t timestamp)
{
	TimeTaggedVOSEvent result;
	result.m_vosEvent = vosEvent;
	result.m_timestamp = timestamp;

	return result;
}
