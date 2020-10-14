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
		kGamepadButton,

		kCount,
	};
}

typedef GpKeyIDSubsets::GpKeyIDSubset GpKeyIDSubset_t;

namespace GpGamepadButtons
{
	enum GpGamepadButton
	{
		kDPadUp,
		kDPadDown,
		kDPadLeft,
		kDPadRight,

		kFaceUp,
		kFaceDown,
		kFaceLeft,
		kFaceRight,

		kLeftStick,
		kRightStick,

		kLeftBumper,
		kRightBumper,

		kMisc1,
		kMisc2,

		kCount,
	};
}

typedef GpGamepadButtons::GpGamepadButton GpGamepadButton_t;

namespace GpGamepadAxes
{
	enum GpGamepadAxis
	{
		kLeftStickX,
		kLeftStickY,

		kRightStickX,
		kRightStickY,

		kLeftTrigger,
		kRightTrigger,

		kCount,
	};
}

typedef GpGamepadAxes::GpGamepadAxis GpGamepadAxis_t;

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
		kPeriod,
		kComma,

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

		kDownChar,
		kAutoChar,
	};
}

typedef GpKeyboardInputEventTypes::GpKeyboardInputEventType GpKeyboardInputEventType_t;

namespace GpGamepadInputEventTypes
{
	enum GpGamepadInputEventType
	{
		kAnalogAxisChanged,
	};
}

typedef GpGamepadInputEventTypes::GpGamepadInputEventType GpGamepadInputEventTypes_t;

struct GpGamepadButtonAndPlayer
{
	GpGamepadButton_t m_button;
	uint8_t m_player;
};

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
		GpGamepadButtonAndPlayer m_gamepadKey;
	};

	GpKeyboardInputEventType_t m_eventType;
	GpKeyIDSubset_t m_keyIDSubset;
	KeyUnion m_key;
	uint32_t m_repeatCount;	// For down and auto events, number of types to repeat this keystroke (if multiple auto-repeated events get compacted)
};

struct GpGamepadAnalogAxisEvent
{
	GpGamepadAxis_t m_axis;
	int16_t m_state;	// Ranges from -32767 to 32767, is never -32768
	uint8_t m_player;
};

struct GpGamepadInputEvent
{
	union EventUnion
	{
		GpGamepadAnalogAxisEvent m_analogAxisEvent;
	};

	GpGamepadInputEventTypes_t m_eventType;
	EventUnion m_event;
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

		kCount
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


namespace GpTouchEventTypes
{
	enum GpTouchEventType
	{
		kUp,
		kDown,
		kMove,
		kLeave,
	};
}

typedef GpTouchEventTypes::GpTouchEventType GpTouchEventType_t;

struct GpTouchInputEvent
{
	int32_t m_x;
	int32_t m_y;
	int64_t m_deviceID;
	int64_t m_fingerID;
	GpTouchEventType_t m_eventType;
};

struct GpVideoResolutionChangedEvent
{
	uint32_t m_prevWidth;
	uint32_t m_prevHeight;
	uint32_t m_newWidth;
	uint32_t m_newHeight;
};

namespace GpVOSEventTypes
{
	enum GpVOSEventType
	{
		kKeyboardInput,
		kMouseInput,
		kTouchInput,
		kGamepadInput,
		kVideoResolutionChanged,
		kQuit
	};
}

typedef GpVOSEventTypes::GpVOSEventType GpVOSEventType_t;

struct GpVOSEvent
{
	union EventUnion
	{
		GpKeyboardInputEvent m_keyboardInputEvent;
		GpMouseInputEvent m_mouseInputEvent;
		GpTouchInputEvent m_touchInputEvent;
		GpGamepadInputEvent m_gamepadInputEvent;
		GpVideoResolutionChangedEvent m_resolutionChangedEvent;
	};

	EventUnion m_event;
	GpVOSEventType_t m_eventType;
};

static const unsigned int GpFKeyMaximumInclusive = 24;
