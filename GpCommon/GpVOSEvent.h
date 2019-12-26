#pragma once

#include <stdint.h>

namespace GpKeyModifiers
{
	enum GpKeyModifier
	{
		kShift,
		kCtrl,
	};
}

namespace GpKeyIDSubsets
{
	enum GpKeyIDSubset
	{
		kASCII,
		kUnicode,
		kSpecial,
		kNumPadNumber,
		kNumPadSpecial,
		kFKey,	// Key value is a raw F number
	};
}

typedef GpKeyIDSubsets::GpKeyIDSubset GpKeyIDSubset_t;

namespace GpKeySpecials
{
	enum GpKeySpecial
	{
		kTab,
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
		kLeftArrow,
		kUpArrow,
		kDownArrow,
		kRightArrow,

		kCount,
	};
}

typedef GpKeySpecials::GpKeySpecial GpKeySpecial_t;


namespace GpNumPadSpecials
{
	enum GpNumPadSpecial
	{
		kSlash,
		kAsterisk,
		kMinus,
		kPlus,

		kCount,
	};
}

typedef GpNumPadSpecials::GpNumPadSpecial GpNumPadSpecial_t;

namespace GpKeyboardInputEventTypes
{
	enum GpKeyboardInputEventType
	{
		kDown,
		kUp,
		kAuto,
	};
}

typedef GpKeyboardInputEventTypes::GpKeyboardInputEventType GpKeyboardInputEventType_t;

struct GpKeyboardInputEvent
{
	union KeyUnion
	{
		GpKeySpecial_t m_specialKey;
		GpNumPadSpecial_t m_numPadSpecialKey;
		uint8_t m_numPadNumber;
		char m_asciiChar;
		uint32_t m_unicodeChar;
		unsigned char m_fKey;
	};

	GpKeyboardInputEventType_t m_eventType;
	GpKeyIDSubset_t m_keyIDSubset;
	KeyUnion m_key;
};

namespace GpMouseEventTypes
{
	enum GpMouseEventType
	{
		kUp,
		kDown,
		kMove,
		kLeave,
	};
}

typedef GpMouseEventTypes::GpMouseEventType GpMouseEventType_t;

namespace GpMouseButtons
{
	enum GpMouseButton
	{
		kNone,
		kLeft,
		kMiddle,
		kRight,
		kX1,
		kX2,
	};
}

typedef GpMouseButtons::GpMouseButton GpMouseButton_t;

struct GpMouseInputEvent
{
	int32_t m_x;
	int32_t m_y;
	GpMouseEventType_t m_eventType;
	GpMouseButton_t m_button;
};

namespace GpVOSEventTypes
{
	enum GpVOSEventType
	{
		kKeyboardInput,
		kMouseInput,
	};
}

typedef GpVOSEventTypes::GpVOSEventType GpVOSEventType_t;

struct GpVOSEvent
{
	union EventUnion
	{
		GpKeyboardInputEvent m_keyboardInputEvent;
		GpMouseInputEvent m_mouseInputEvent;
	};

	EventUnion m_event;
	GpVOSEventType_t m_eventType;
};

static const unsigned int GpFKeyMaximumInclusive = 24;
