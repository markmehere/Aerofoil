#pragma once

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
		GpKeySpecials::GpKeySpecial m_specialKey;
		char m_asciiChar;
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
