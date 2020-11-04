//============================================================================
//----------------------------------------------------------------------------
//								GliderStructs.h
//----------------------------------------------------------------------------
//============================================================================
#pragma once

#include "GliderDefines.h"
#include "PLQDOffscreen.h"
#include "GpVOSEvent.h"
#include "ByteSwap.h"

typedef struct
{
	Point		topLeft;				// 4
	int16_t		distance;				// 2
	Boolean		initial;				// 1
	Boolean		state;					// 1		              F. lf. dn. rt. up
	Byte		vector;					// 1		| x | x | x | x | 8 | 4 | 2 | 1 |
	Byte		tall;					// 1
} blowerType;							// total = 10

typedef struct
{
	Rect		bounds;					// 8
	int16_t		pict;					// 2
} furnitureType;						// total = 10

typedef struct
{
	Point		topLeft;				// 4
	int16_t		length;					// 2 grease spill
	int16_t		points;					// 2 invis bonus
	Boolean		state;					// 1
	Boolean		initial;				// 1
} bonusType;							// total = 10

typedef struct
{
	Point		topLeft;				// 4
	int16_t		tall;					// 2 invis transport
	int16_t		where;					// 2
	Byte		who;					// 1
	Byte		wide;					// 1
} transportType;						// total = 10

typedef struct
{
	Point		topLeft;				// 4
	int16_t		delay;					// 2
	int16_t		where;					// 2
	Byte		who;					// 1
	Byte		type;					// 1
} switchType;							// total = 10

typedef struct
{
	Point		topLeft;				// 4
	int16_t		length;					// 2
	Byte		byte0;					// 1
	Byte		byte1;					// 1
	Boolean		initial;				// 1
	Boolean		state;					// 1
} lightType;							// total = 10

typedef struct
{
	Point		topLeft;				// 4
	int16_t		height;					// 2 toaster, pict ID
	Byte		byte0;					// 1
	Byte		delay;					// 1
	Boolean		initial;				// 1
	Boolean		state;					// 1
} applianceType;						// total = 10

typedef struct
{
	Point		topLeft;					// 4
	int16_t		length;						// 2
	Byte		delay;						// 1
	Byte		byte0;						// 1
	Boolean		initial;					// 1
	Boolean		state;						// 1
} enemyType;								// total = 10

typedef struct
{
	Rect		bounds;						// 8
	int16_t		pict;						// 2
} clutterType;								// total = 10

typedef struct
{
	int16_t		what;						// 2
	union
	{
		blowerType		a;
		furnitureType	b;
		bonusType		c;
		transportType	d;
		switchType		e;
		lightType		f;
		applianceType	g;
		enemyType		h;
		clutterType		i;
	} data;									// 10
} objectType, *objectPtr;					// total = 12

typedef struct
{
	Str31			banner;					// 32		= 32
	Str15			names[kMaxScores];		// 16 * 10	= 160
	Int32			scores[kMaxScores];		// 4 * 10	= 40
	UInt32			timeStamps[kMaxScores];	// 4 * 10	= 40
	int16_t			levels[kMaxScores];		// 2 * 10	= 20
} scoresType;								// total 	= 292

typedef struct
{
	int16_t		version;					// 2
	int16_t		wasStarsLeft;				// 2
	UInt32		timeStamp;					// 4
	Point		where;						// 4
	Int32		score;						// 4
	Int32		unusedLong;					// 4
	Int32		unusedLong2;				// 4
	int16_t		energy;						// 2
	int16_t		bands;						// 2
	int16_t		roomNumber;					// 2
	int16_t		gliderState;				// 2
	int16_t		numGliders;					// 2
	int16_t		foil;						// 2
	int16_t		unusedShort;				// 2
	Boolean		facing;						// 1
	Boolean		showFoil;					// 1
} gameType;									// total = 40

typedef struct
{
	short		unusedShort;				// 2
	Byte		unusedByte;					// 1
	Boolean		visited;					// 1
	objectType	objects[kMaxRoomObs];		// 24 * 12
} savedRoom, *saveRoomPtr;					// total = 292

typedef struct
{
	VFileSpec	house;						// 70
	short		version;					// 2
	short		wasStarsLeft;				// 2
	long		timeStamp;					// 4
	Point		where;						// 4
	long		score;						// 4
	long		unusedLong;					// 4
	long		unusedLong2;				// 4
	short		energy;						// 2
	short		bands;						// 2
	short		roomNumber;					// 2
	short		gliderState;				// 2
	short		numGliders;					// 2
	short		foil;						// 2
	short		nRooms;						// 2
	Boolean		facing;						// 1
	Boolean		showFoil;					// 1
	savedRoom	savedData[1];				// 4
} game2Type, *gamePtr;						// total = 114

typedef struct
{
	Str27		name;						// 28
	int16_t		bounds;						// 2
	Byte		leftStart;					// 1
	Byte		rightStart;					// 1
	Byte		unusedByte;					// 1
	Boolean		visited;					// 1
	int16_t		background;					// 2
	int16_t		tiles[kNumTiles];			// 2 * 8
	int16_t		floor, suite;				// 2 + 2
	int16_t		openings;					// 2
	int16_t		numObjects;					// 2
	objectType	objects[kMaxRoomObs];		// 24 * 12
} roomType, *roomPtr;						// total = 348

struct houseType
{
	int16_t		version;					// 2
	int16_t		unusedShort;				// 2
	int32_t		timeStamp;					// 4
	int32_t		flags;						// 4 (bit 0 = wardBit)
	Point		initial;					// 4
	Str255		banner;						// 256
	Str255		trailer;					// 256
	scoresType	highScores;					// 292
	gameType	savedGame;					// 40
	Boolean		hasGame;					// 1
	Boolean		unusedBoolean;				// 1
	int16_t		firstRoom;					// 2
	int16_t		nRooms;						// 2

	int16_t		padding;
	roomType	rooms[1];					// 348 * nRooms
											// total = 866 +

	static const size_t kBinaryDataSize = 866;
};

typedef houseType *housePtr;

typedef THandle<houseType> houseHand;

typedef struct
{
	Rect		src, mask, dest, whole;
	Rect		destShadow, wholeShadow;
	Rect		clip, enteredRect;
	Int32		leftKey, rightKey;
	Int32		battKey, bandKey;
	Int32		gamepadLeftKey, gamepadRightKey;
	Int32		gamepadBattKey, gamepadBandKey;
	Int32		gamepadFlipKey;
	Int32		gamepadFaceLeftKey, gamepadFaceRightKey;
	short		hVel, vVel;
	short		wasHVel, wasVVel;
	short		vDesiredVel, hDesiredVel;
	short		mode, frame, wasMode;
	Boolean		facing, tipped;
	Boolean		sliding, ignoreLeft, ignoreRight;
	Boolean		fireHeld, which;
	Boolean		heldLeft, heldRight, heldFlip;
	Boolean		dontDraw, ignoreGround;
} gliderType, *gliderPtr;

typedef struct
{
	Rect		bounds;
	short		action;
	short		who;
	Boolean		isOn, stillOver;
	Boolean		doScrutinize;
} hotObject, *hotPtr;

typedef struct
{
	Rect			dest;
	DrawSurface		*map;
	short			where;
	short			who;
	short			component;
} savedType, *savedPtr;

typedef struct
{
	Rect		bounds;
	short		mode;
} sparkleType, *sparklePtr;

typedef struct
{
	Rect		dest, whole;
	short		start;
	short		stop;
	short		mode;
	short		loops;
	short		hVel, vVel;
} flyingPtType, *flyingPtPtr;

typedef struct
{
	Rect		dest, src;
	short		mode;
	short		who;
} flameType, *flamePtr;

typedef struct
{
	Rect		dest, src;
	short		mode, where;
	short		who, link;
	Boolean		toOrFro, active;
} pendulumType, *pendulumPtr;

typedef struct
{
	Boolean		left;
	Boolean		top;
	Boolean		right;
	Boolean		bottom;
} boundsType, *boundsPtr;

typedef THandle<boundsType> boundsHand;


typedef struct
{
	Rect		dest;
	short		mode, count;
	short		hVel, vVel;
} bandType, *bandPtr;

typedef struct
{
	short		srcRoom, srcObj;
	short		destRoom, destObj;
} linksType, *linksPtr;

typedef struct
{
	Rect		dest;
	short		mapNum, mode;
	short		who, where;
	short		start, stop;
	short		frame, hotNum;
	Boolean		isRight;
} greaseType, *greasePtr;

typedef struct
{
	Rect		dest, src;
	short		mode, who;
	short		link, where;
} starType, *starPtr;

typedef struct
{
	Rect		bounds;
	short		frame;
} shredType, *shredPtr;

typedef struct
{
	Rect		dest;
	Rect		whole;
	short		hVel, vVel;
	short		type, count;
	short		frame, timer;
	short		position, room;
	Byte		byte0, byte1;
	Boolean		moving, active;
} dynaType, *dynaPtr;

typedef struct
{
	short		roomNum;	// room # object in (real number)
	short		objectNum;	// obj. # in house (real number)
	short		roomLink;	// room # object linked to (if any)
	short		objectLink;	// obj. # object linked to (if any)
	short		localLink;	// index in master list if exists
	short		hotNum;		// index into active rects (if any)
	short		dynaNum;	// index into dinahs (if any)
	objectType	theObject;	// actual object data
} objDataType, *objDataPtr;

typedef struct
{
	BEInt32_t	frame;
	char		key;
	char		padding;
} demoType, *demoPtr;

typedef struct
{
	short		room;
	short		object;
} retroLink, *retroLinkPtr;

namespace TouchScreenCtrlIDs
{
	enum TouchScreenCtrlID
	{
		Bands,
		BatteryHelium,
		Menu,

		Movement,	// Keep this last

		Count,

		Invalid,
	};
};

typedef TouchScreenCtrlIDs::TouchScreenCtrlID TouchScreenCtrlID_t;

struct touchScreenFingerID
{
	int64_t m_deviceID;
	int64_t m_fingerID;

	touchScreenFingerID();
	touchScreenFingerID(int64_t deviceID, int64_t fingerID);

	bool operator ==(const touchScreenFingerID &other) const;
	bool operator !=(const touchScreenFingerID &other) const;
};

inline touchScreenFingerID::touchScreenFingerID()
	: m_fingerID(0)
	, m_deviceID(0)
{
}

inline touchScreenFingerID::touchScreenFingerID(int64_t deviceID, int64_t fingerID)
	: m_fingerID(fingerID)
	, m_deviceID(deviceID)
{
}

inline bool touchScreenFingerID::operator==(const touchScreenFingerID &other) const
{
	return this->m_fingerID == other.m_fingerID && this->m_deviceID == other.m_deviceID;
}

inline bool touchScreenFingerID::operator!=(const touchScreenFingerID &other) const
{
	return !((*this) == other);
}

typedef struct
{
	touchScreenFingerID tfingerID;
	Point point;
	TouchScreenCtrlID_t capturingControl;
	bool active;
} touchScreenFingerState;

typedef struct
{
	Rect graphicRect;
	Rect touchRect;
	Boolean isEnabled;
} touchScreenControl;

namespace touchScreenControlGraphics
{
	enum touchScreenControlGraphic
	{
		MenuActive,
		MenuIdle,
		BandsDisabled,
		BandsActive,
		BandsIdle,
		StopActive,
		StopIdle,
		MoveRightActive,
		MoveRightIdle,
		MoveLeftActive,
		MoveLeftIdle,
		HeliumDisabled,
		HeliumActive,
		HeliumIdle,
		BatteryDisabled,
		BatteryActive,
		BatteryIdle,

		Count,
	};

	static const int kTouchScreenGraphicStartID = 1971;
}

typedef touchScreenControlGraphics::touchScreenControlGraphic touchScreenControlGraphic_t;

struct touchScreenControlState
{
	static const int kMaxFingers = 4;

	touchScreenControl controls[TouchScreenCtrlIDs::Count];
	touchScreenFingerState fingers[kMaxFingers];

	DrawSurface *graphics[touchScreenControlGraphics::Count];
};

typedef touchScreenControlState *touchScreenControlStatePtr;
