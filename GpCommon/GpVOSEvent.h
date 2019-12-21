#pragma once

namespace GpKeyIDSubsets
{
	enum GpKeyIDSubset
	{
		kASCII,
		kSpecial,
		kNumPadASCII,
		kNumPadSpecial,
		kFKey,	// Key value is a raw F number
	};
}

typedef GpKeyIDSubsets::GpKeyIDSubset GpKeyIDSubset_t;

namespace GpKeySpecials
{
	enum GpKeySpecial
	{
		kEscape,
		kPrintScreen,
		kScrollLock,
		kPause,
		kInsert,
		kHome,
		kPageUp,
		kPageDown,
		kDelete,
		kEnd,
		kBackspace,
		kCapsLock,
		kEnter,
		kLeftShift,
		kRightShift,
		kLeftCtrl,
		kRightCtrl,
		kLeftAlt,
		kRightAlt,
		kNumLock,
	};
}

typedef GpKeySpecials::GpKeySpecial GpKeySpecial_t;

namespace GpInputEventTypes
{
	enum GpInputEventType
	{
		kDown,
		kUp,
		kAuto,
	};
}

typedef GpInputEventTypes::GpInputEventType GpInputEventType_t;

namespace GpVOSEventTypes
{
	enum GpVOSEventType
	{
		kInput,
	};
}

typedef GpVOSEventTypes::GpVOSEventType GpVOSEventType_t;

struct GpInputEvent
{
	union KeyUnion
	{
		GpKeySpecials::GpKeySpecial m_specialKey;
		char m_asciiChar;
	};

	GpInputEventType_t m_eventType;
	GpKeyIDSubset_t m_keyIDSubset;
	KeyUnion m_key;
};

struct GpVOSEvent
{
	union EventUnion
	{
		GpInputEvent m_inputEvent;
	};

	GpVOSEventType_t m_eventType;
};
