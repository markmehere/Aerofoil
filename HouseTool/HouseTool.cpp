#include <stdint.h>
#include "PLCore.h"
#include "PLBigEndian.h"
#include "MacRomanConversion.h"
#include "GpUnicode.h"
#include "WindowsUnicodeToolShim.h"

#include <vector>

typedef uint8_t Boolean;

#define kMaxScores					10
#define kMaxRoomObs					24
#define kNumTiles					8

// Object types

#define kFloorVent					0x01		// Blowers
#define kCeilingVent				0x02
#define kFloorBlower				0x03
#define kCeilingBlower				0x04
#define kSewerGrate					0x05
#define kLeftFan					0x06
#define kRightFan					0x07
#define kTaper						0x08
#define kCandle						0x09
#define kStubby						0x0A
#define kTiki						0x0B
#define kBBQ						0x0C
#define kInvisBlower				0x0D
#define kGrecoVent					0x0E
#define kSewerBlower				0x0F
#define kLiftArea					0x10

#define kTable						0x11		// Furniture
#define kShelf						0x12
#define kCabinet					0x13
#define kFilingCabinet				0x14
#define kWasteBasket				0x15
#define kMilkCrate					0x16
#define kCounter					0x17
#define kDresser					0x18
#define kDeckTable					0x19
#define kStool						0x1A
#define kTrunk						0x1B
#define kInvisObstacle				0x1C
#define kManhole					0x1D
#define kBooks						0x1E
#define kInvisBounce				0x1F

#define kRedClock					0x21		// Prizes
#define kBlueClock					0x22
#define kYellowClock				0x23
#define kCuckoo						0x24
#define kPaper						0x25
#define kBattery					0x26
#define kBands						0x27
#define kGreaseRt					0x28
#define kGreaseLf					0x29
#define kFoil						0x2A
#define kInvisBonus					0x2B
#define kStar						0x2C
#define kSparkle					0x2D
#define kHelium						0x2E
#define kSlider						0x2F

#define kUpStairs					0x31		// Transport
#define kDownStairs					0x32
#define kMailboxLf					0x33
#define kMailboxRt					0x34
#define kFloorTrans					0x35
#define kCeilingTrans				0x36
#define kDoorInLf					0x37
#define kDoorInRt					0x38
#define kDoorExRt					0x39
#define kDoorExLf					0x3A
#define kWindowInLf					0x3B
#define kWindowInRt					0x3C
#define kWindowExRt					0x3D
#define kWindowExLf					0x3E
#define kInvisTrans					0x3F
#define kDeluxeTrans				0x40

#define kLightSwitch				0x41		// Switches
#define kMachineSwitch				0x42
#define kThermostat					0x43
#define kPowerSwitch				0x44
#define kKnifeSwitch				0x45
#define kInvisSwitch				0x46
#define kTrigger					0x47
#define kLgTrigger					0x48
#define kSoundTrigger				0x49

#define kCeilingLight				0x51		// Lights
#define kLightBulb					0x52
#define kTableLamp					0x53
#define kHipLamp					0x54
#define kDecoLamp					0x55
#define kFlourescent				0x56
#define kTrackLight					0x57
#define kInvisLight					0x58

#define kShredder					0x61		// Appliances
#define kToaster					0x62
#define kMacPlus					0x63
#define kGuitar						0x64
#define kTV							0x65
#define kCoffee						0x66
#define kOutlet						0x67
#define kVCR						0x68
#define kStereo						0x69
#define kMicrowave					0x6A
#define kCinderBlock				0x6B
#define kFlowerBox					0x6C
#define kCDs						0x6D
#define kCustomPict					0x6E

#define kBalloon					0x71		// Enemies
#define kCopterLf					0x72
#define kCopterRt					0x73
#define kDartLf						0x74
#define kDartRt						0x75
#define kBall						0x76
#define kDrip						0x77
#define kFish						0x78
#define kCobweb						0x79

#define kOzma						0x81		// Clutter
#define kMirror						0x82
#define kMousehole					0x83
#define kFireplace					0x84
#define kFlower						0x85
#define kWallWindow					0x86
#define kBear						0x87
#define kCalendar					0x88
#define kVase1						0x89
#define kVase2						0x8A
#define kBulletin					0x8B
#define kCloud						0x8C
#define kFaucet						0x8D
#define kRug						0x8E
#define kChimes						0x8F


struct EnumDef
{
	int m_value;
	const char *m_name;
};

struct Bool8
{
	uint8_t m_value;
};

template<class T, int TSize>
struct StaticArray
{
	static const int kSize = TSize;

	T m_data[TSize];
};


template<int TSize>
struct PascalStr
{
	static const int kSize = TSize;

	uint8_t m_length;
	uint8_t m_chars[TSize];
};

typedef PascalStr<15> PStr15_t;
typedef PascalStr<27> PStr27_t;
typedef PascalStr<31> PStr31_t;
typedef PascalStr<255> PStr255_t;



typedef struct
{
	BEPoint		topLeft;				// 4
	BEInt16_t	distance;				// 2
	Bool8		initial;				// 1
	Bool8		state;					// 1		              F. lf. dn. rt. up
	uint8_t		vector;					// 1		| x | x | x | x | 8 | 4 | 2 | 1 |
	uint8_t		tall;					// 1
} blowerType;							// total = 10

typedef struct
{
	BERect		bounds;					// 8
	BEInt16_t	pict;					// 2
} furnitureType;						// total = 10

typedef struct
{
	BEPoint		topLeft;				// 4
	BEInt16_t	length;					// 2 grease spill
	BEInt16_t	points;					// 2 invis bonus
	Bool8		state;					// 1
	Bool8		initial;				// 1
} bonusType;							// total = 10

typedef struct
{
	BEPoint		topLeft;				// 4
	BEInt16_t	tall;					// 2 invis transport
	BEInt16_t	where;					// 2
	uint8_t		who;					// 1
	uint8_t		wide;					// 1
} transportType;						// total = 10

typedef struct
{
	BEPoint		topLeft;				// 4
	BEInt16_t	delay;					// 2
	BEInt16_t	where;					// 2
	uint8_t		who;					// 1
	uint8_t		type;					// 1
} switchType;							// total = 10

typedef struct
{
	BEPoint		topLeft;				// 4
	BEInt16_t	length;					// 2
	uint8_t		byte0;					// 1
	uint8_t		byte1;					// 1
	Bool8		initial;				// 1
	Bool8		state;					// 1
} lightType;							// total = 10

typedef struct
{
	BEPoint		topLeft;				// 4
	BEInt16_t	height;					// 2 toaster, pict ID
	uint8_t		byte0;					// 1
	uint8_t		delay;					// 1
	Bool8		initial;				// 1
	Bool8		state;					// 1
} applianceType;						// total = 10

typedef struct
{
	BEPoint		topLeft;					// 4
	BEInt16_t	length;						// 2
	uint8_t		delay;						// 1
	uint8_t		byte0;						// 1
	Bool8		initial;					// 1
	Bool8		state;						// 1
} enemyType;								// total = 10

typedef struct
{
	BERect		bounds;						// 8
	BEInt16_t	pict;						// 2
} clutterType;								// total = 10

struct objectType
{
	union u
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
		StaticArray<uint8_t, 10> pad;

		u();
		u(const u &other);
		~u();
	};

	BEInt16_t	what;						// 2
	u			data;						// 10
};											// total = 12

objectType::u::u()
{
	static_assert(sizeof(*this) == 10, "Wrong size");
}

objectType::u::~u()
{
}

objectType::u::u(const u &other)
{
	memcpy(this, &other, sizeof(*this));
}

typedef objectType *objectPtr;

typedef struct
{
	PStr31_t		banner;					// 32		= 32
	StaticArray<PStr15_t, kMaxScores>		names;		// 16 * 10	= 160
	StaticArray<BEInt32_t, kMaxScores>		scores;		// 4 * 10	= 40
	StaticArray<BEUInt32_t, kMaxScores>		timeStamps;	// 4 * 10	= 40
	StaticArray<BEInt16_t, kMaxScores>		levels;		// 2 * 10	= 20
} scoresType;								// total 	= 292

typedef struct
{
	BEInt16_t	version;					// 2
	BEInt16_t	wasStarsLeft;				// 2
	BEUInt32_t	timeStamp;					// 4
	BEPoint		where;						// 4
	BEInt32_t	score;						// 4
	BEInt32_t	unusedLong;					// 4
	BEInt32_t	unusedLong2;				// 4
	BEInt16_t	energy;						// 2
	BEInt16_t	bands;						// 2
	BEInt16_t	roomNumber;					// 2
	BEInt16_t	gliderState;				// 2
	BEInt16_t	numGliders;					// 2
	BEInt16_t	foil;						// 2
	BEInt16_t	unusedShort;				// 2
	Bool8		facing;						// 1
	Bool8		showFoil;					// 1
} gameType;									// total = 40

typedef struct
{
	BEInt16_t	unusedShort;				// 2
	uint8_t		unusedByte;					// 1
	Bool8		visited;					// 1
	objectType	objects[kMaxRoomObs];		// 24 * 12
} savedRoom, *saveRoomPtr;					// total = 292

typedef struct
{
	PStr27_t	name;								// 28
	BEInt16_t	bounds;								// 2
	uint8_t		leftStart;							// 1
	uint8_t		rightStart;							// 1
	uint8_t		unusedByte;							// 1
	Bool8		visited;							// 1
	BEInt16_t	background;							// 2
	StaticArray<BEInt16_t, kNumTiles>	tiles;		// 2 * 8
	BEInt16_t	floor, suite;						// 2 + 2
	BEInt16_t	openings;							// 2
	BEInt16_t	numObjects;							// 2
	StaticArray<objectType, kMaxRoomObs> objects;	// 24 * 12
} roomType, *roomPtr;								// total = 348

struct houseType
{
	BEInt16_t	version;					// 2
	BEInt16_t	unusedShort;				// 2
	BEInt32_t	timeStamp;					// 4
	BEInt32_t	flags;						// 4 (bit 0 = wardBit)
	BEPoint		initial;					// 4
	PStr255_t	banner;						// 256
	PStr255_t	trailer;					// 256
	scoresType	highScores;					// 292
	gameType	savedGame_Unused;			// 40
	Bool8		hasGame;					// 1
	Bool8		unusedBoolean;				// 1
	BEInt16_t	firstRoom;					// 2
	BEInt16_t	nRooms;						// 2

	static const size_t kBinaryDataSize = 866;
};

static const size_t houseSize = sizeof(houseType);

#define ENUM_VALUE_DEF(k) { k, #k }

EnumDef g_objectTypeEnum[] =
{
	{ -1, "kObjectIsEmpty" },
	ENUM_VALUE_DEF(kFloorVent),
	ENUM_VALUE_DEF(kCeilingVent),
	ENUM_VALUE_DEF(kFloorBlower),
	ENUM_VALUE_DEF(kCeilingBlower),
	ENUM_VALUE_DEF(kSewerGrate),
	ENUM_VALUE_DEF(kLeftFan),
	ENUM_VALUE_DEF(kRightFan),
	ENUM_VALUE_DEF(kTaper),
	ENUM_VALUE_DEF(kCandle),
	ENUM_VALUE_DEF(kStubby),
	ENUM_VALUE_DEF(kTiki),
	ENUM_VALUE_DEF(kBBQ),
	ENUM_VALUE_DEF(kInvisBlower),
	ENUM_VALUE_DEF(kGrecoVent),
	ENUM_VALUE_DEF(kSewerBlower),
	ENUM_VALUE_DEF(kLiftArea),
	ENUM_VALUE_DEF(kTable),
	ENUM_VALUE_DEF(kShelf),
	ENUM_VALUE_DEF(kCabinet),
	ENUM_VALUE_DEF(kFilingCabinet),
	ENUM_VALUE_DEF(kWasteBasket),
	ENUM_VALUE_DEF(kMilkCrate),
	ENUM_VALUE_DEF(kCounter),
	ENUM_VALUE_DEF(kDresser),
	ENUM_VALUE_DEF(kDeckTable),
	ENUM_VALUE_DEF(kStool),
	ENUM_VALUE_DEF(kTrunk),
	ENUM_VALUE_DEF(kInvisObstacle),
	ENUM_VALUE_DEF(kManhole),
	ENUM_VALUE_DEF(kBooks),
	ENUM_VALUE_DEF(kInvisBounce),
	ENUM_VALUE_DEF(kRedClock),
	ENUM_VALUE_DEF(kBlueClock),
	ENUM_VALUE_DEF(kYellowClock),
	ENUM_VALUE_DEF(kCuckoo),
	ENUM_VALUE_DEF(kPaper),
	ENUM_VALUE_DEF(kBattery),
	ENUM_VALUE_DEF(kBands),
	ENUM_VALUE_DEF(kGreaseRt),
	ENUM_VALUE_DEF(kGreaseLf),
	ENUM_VALUE_DEF(kFoil),
	ENUM_VALUE_DEF(kInvisBonus),
	ENUM_VALUE_DEF(kStar),
	ENUM_VALUE_DEF(kSparkle),
	ENUM_VALUE_DEF(kHelium),
	ENUM_VALUE_DEF(kSlider),
	ENUM_VALUE_DEF(kUpStairs),
	ENUM_VALUE_DEF(kDownStairs),
	ENUM_VALUE_DEF(kMailboxLf),
	ENUM_VALUE_DEF(kMailboxRt),
	ENUM_VALUE_DEF(kFloorTrans),
	ENUM_VALUE_DEF(kCeilingTrans),
	ENUM_VALUE_DEF(kDoorInLf),
	ENUM_VALUE_DEF(kDoorInRt),
	ENUM_VALUE_DEF(kDoorExRt),
	ENUM_VALUE_DEF(kDoorExLf),
	ENUM_VALUE_DEF(kWindowInLf),
	ENUM_VALUE_DEF(kWindowInRt),
	ENUM_VALUE_DEF(kWindowExRt),
	ENUM_VALUE_DEF(kWindowExLf),
	ENUM_VALUE_DEF(kInvisTrans),
	ENUM_VALUE_DEF(kDeluxeTrans),
	ENUM_VALUE_DEF(kLightSwitch),
	ENUM_VALUE_DEF(kMachineSwitch),
	ENUM_VALUE_DEF(kThermostat),
	ENUM_VALUE_DEF(kPowerSwitch),
	ENUM_VALUE_DEF(kKnifeSwitch),
	ENUM_VALUE_DEF(kInvisSwitch),
	ENUM_VALUE_DEF(kTrigger),
	ENUM_VALUE_DEF(kLgTrigger),
	ENUM_VALUE_DEF(kSoundTrigger),
	ENUM_VALUE_DEF(kCeilingLight),
	ENUM_VALUE_DEF(kLightBulb),
	ENUM_VALUE_DEF(kTableLamp),
	ENUM_VALUE_DEF(kHipLamp),
	ENUM_VALUE_DEF(kDecoLamp),
	ENUM_VALUE_DEF(kFlourescent),
	ENUM_VALUE_DEF(kTrackLight),
	ENUM_VALUE_DEF(kInvisLight),
	ENUM_VALUE_DEF(kShredder),
	ENUM_VALUE_DEF(kToaster),
	ENUM_VALUE_DEF(kMacPlus),
	ENUM_VALUE_DEF(kGuitar),
	ENUM_VALUE_DEF(kTV),
	ENUM_VALUE_DEF(kCoffee),
	ENUM_VALUE_DEF(kOutlet),
	ENUM_VALUE_DEF(kVCR),
	ENUM_VALUE_DEF(kStereo),
	ENUM_VALUE_DEF(kMicrowave),
	ENUM_VALUE_DEF(kCinderBlock),
	ENUM_VALUE_DEF(kFlowerBox),
	ENUM_VALUE_DEF(kCDs),
	ENUM_VALUE_DEF(kCustomPict),
	ENUM_VALUE_DEF(kBalloon),
	ENUM_VALUE_DEF(kCopterLf),
	ENUM_VALUE_DEF(kCopterRt),
	ENUM_VALUE_DEF(kDartLf),
	ENUM_VALUE_DEF(kDartRt),
	ENUM_VALUE_DEF(kBall),
	ENUM_VALUE_DEF(kDrip),
	ENUM_VALUE_DEF(kFish),
	ENUM_VALUE_DEF(kCobweb),
	ENUM_VALUE_DEF(kOzma),
	ENUM_VALUE_DEF(kMirror),
	ENUM_VALUE_DEF(kMousehole),
	ENUM_VALUE_DEF(kFireplace),
	ENUM_VALUE_DEF(kFlower),
	ENUM_VALUE_DEF(kWallWindow),
	ENUM_VALUE_DEF(kBear),
	ENUM_VALUE_DEF(kCalendar),
	ENUM_VALUE_DEF(kVase1),
	ENUM_VALUE_DEF(kVase2),
	ENUM_VALUE_DEF(kBulletin),
	ENUM_VALUE_DEF(kCloud),
	ENUM_VALUE_DEF(kFaucet),
	ENUM_VALUE_DEF(kRug),
	ENUM_VALUE_DEF(kChimes),
};

const size_t g_objectTypeEnumSize = sizeof(g_objectTypeEnum) / sizeof(g_objectTypeEnum[0]);


struct IDataVisitor
{
	virtual IDataVisitor *EnterIndex(int index) = 0;
	virtual IDataVisitor *EnterNamed(const char *name) = 0;
	virtual void Exit() = 0;

	virtual void VisitLPStr(uint8_t &length, uint8_t *chars, int capacity) = 0;
	virtual void VisitInt32(int32_t &v) = 0;
	virtual void VisitUInt32(uint32_t &v) = 0;
	virtual void VisitInt16(int16_t &v) = 0;
	virtual void VisitBool8(uint8_t &v) = 0;
	virtual void VisitByte(uint8_t &v) = 0;
	virtual bool VisitEnum(const EnumDef *enumDefs, size_t numDefs, int &v) = 0;

	virtual bool VisitStaticCapacity(size_t size) = 0;
	virtual void VisitDynamicCapacity(size_t &size) = 0;
};

bool VisitEnum(const EnumDef *enumDefs, size_t numDefs, IDataVisitor *visitor);

void Visit(roomType &v, IDataVisitor *visitor);
void Visit(objectType &v, IDataVisitor *visitor);
void Visit(blowerType &v, IDataVisitor *visitor);
void Visit(furnitureType &v, IDataVisitor *visitor);
void Visit(bonusType &v, IDataVisitor *visitor);
void Visit(transportType &v, IDataVisitor *visitor);
void Visit(switchType &v, IDataVisitor *visitor);
void Visit(lightType &v, IDataVisitor *visitor);
void Visit(applianceType &v, IDataVisitor *visitor);
void Visit(enemyType &v, IDataVisitor *visitor);
void Visit(clutterType &v, IDataVisitor *visitor);
void Visit(scoresType &v, IDataVisitor *visitor);
void Visit(gameType &v, IDataVisitor *visitor);
void Visit(Bool8 &v, IDataVisitor *visitor);
void Visit(uint8_t &v, IDataVisitor *visitor);
void Visit(BEInt16_t &v, IDataVisitor *visitor);
void Visit(BEInt32_t &v, IDataVisitor *visitor);
void Visit(BEUInt32_t &v, IDataVisitor *visitor);
void Visit(BEPoint &v, IDataVisitor *visitor);
void Visit(BERect &v, IDataVisitor *visitor);

template<int TSize>
void Visit(PascalStr<TSize> &pstr, IDataVisitor *visitor)
{
	visitor->VisitLPStr(pstr.m_length, pstr.m_chars, TSize);
}

template<class T, int TSize>
void Visit(StaticArray<T, TSize> &arr, IDataVisitor *visitor)
{
	if (!visitor->VisitStaticCapacity(TSize))
		return;

	for (int i = 0; i < TSize; i++)
	{
		IDataVisitor *subscriptVisitor = visitor->EnterIndex(i);
		if (subscriptVisitor)
		{
			Visit(arr.m_data[i], visitor);
			subscriptVisitor->Exit();
		}
	}
}

template<class T>
void VisitField(const char *subscriptName, T &subscript, IDataVisitor *visitor)
{
	IDataVisitor *subscriptVisitor = visitor->EnterNamed(subscriptName);
	if (subscriptVisitor)
	{
		Visit(subscript, visitor);
		subscriptVisitor->Exit();
	}
}

template<class T>
void VisitArray(T *arr, int size, IDataVisitor *visitor)
{
	if (!visitor->VisitStaticCapacity(static_cast<size_t>(size)))
		return;

	for (int i = 0; i < size; i++)
	{
		IDataVisitor *subscriptVisitor = visitor->EnterIndex(i);
		if (subscriptVisitor)
		{
			Visit(arr[i], visitor);
			subscriptVisitor->Exit();
		}
	}
}


bool VisitEnum(const EnumDef *enumDefs, size_t numDefs, int &v, IDataVisitor *visitor)
{
	return visitor->VisitEnum(enumDefs, numDefs, v);
}

void Visit(houseType &house, std::vector<roomType> &rooms, IDataVisitor *visitor)
{
	VisitField("version", house.version, visitor);
	VisitField("unusedShort", house.unusedShort, visitor);
	VisitField("timeStamp", house.timeStamp, visitor);
	VisitField("flags", house.flags, visitor);
	VisitField("initial", house.initial, visitor);
	VisitField("banner", house.banner, visitor);
	VisitField("trailer", house.trailer, visitor);
	VisitField("highScores", house.highScores, visitor);
	VisitField("savedGame", house.savedGame_Unused, visitor);
	VisitField("hasGame", house.hasGame, visitor);
	VisitField("unusedBoolean", house.unusedBoolean, visitor);
	VisitField("firstRoom", house.firstRoom, visitor);
	VisitField("nRooms", house.nRooms, visitor);

	IDataVisitor *roomsSubscript = visitor->EnterNamed("rooms");
	if (roomsSubscript)
	{
		size_t size = rooms.size();
		roomsSubscript->VisitDynamicCapacity(size);

		if (size > 0x7fff)
			size = 0x7fff;

		if (size < rooms.size())
			rooms.resize(size);
		else if (size > rooms.size())
		{
			roomType emptyRoom;
			memset(&emptyRoom, 0, sizeof(emptyRoom));

			rooms.reserve(size);
			while (size > rooms.size())
				rooms.push_back(emptyRoom);
		}

		house.nRooms = static_cast<int16_t>(size);

		for (size_t i = 0; i < size; i++)
		{
			IDataVisitor *roomSubscript = roomsSubscript->EnterIndex(static_cast<int>(i));
			if (roomSubscript)
			{
				Visit(rooms[i], roomSubscript);
				roomSubscript->Exit();
			}
		}

		roomsSubscript->Exit();
	}
}

void Visit(blowerType &v, IDataVisitor *visitor)
{
	VisitField("topLeft", v.topLeft, visitor);
	VisitField("distance", v.distance, visitor);
	VisitField("initial", v.initial, visitor);
	VisitField("state", v.state, visitor);
	VisitField("vector", v.vector, visitor);
	VisitField("tall", v.tall, visitor);
}

void Visit(furnitureType &v, IDataVisitor *visitor)
{
	VisitField("bounds", v.bounds, visitor);
	VisitField("pict", v.pict, visitor);
}

void Visit(bonusType &v, IDataVisitor *visitor)
{
	VisitField("topLeft", v.topLeft, visitor);
	VisitField("length", v.length, visitor);
	VisitField("points", v.points, visitor);
	VisitField("state", v.state, visitor);
	VisitField("initial", v.initial, visitor);
}

void Visit(transportType &v, IDataVisitor *visitor)
{
	VisitField("topLeft", v.topLeft, visitor);
	VisitField("tall", v.tall, visitor);
	VisitField("where", v.where, visitor);
	VisitField("who", v.who, visitor);
	VisitField("wide", v.wide, visitor);
}

void Visit(switchType &v, IDataVisitor *visitor)
{
	VisitField("topLeft", v.topLeft, visitor);
	VisitField("delay", v.delay, visitor);
	VisitField("where", v.where, visitor);
	VisitField("who", v.who, visitor);
	VisitField("type", v.type, visitor);
}

void Visit(lightType &v, IDataVisitor *visitor)
{
	VisitField("topLeft", v.topLeft, visitor);
	VisitField("length", v.length, visitor);
	VisitField("byte0", v.byte0, visitor);
	VisitField("byte1", v.byte1, visitor);
	VisitField("initial", v.initial, visitor);
	VisitField("state", v.state, visitor);
}

void Visit(applianceType &v, IDataVisitor *visitor)
{
	VisitField("topLeft", v.topLeft, visitor);
	VisitField("height", v.height, visitor);
	VisitField("byte0", v.byte0, visitor);
	VisitField("delay", v.delay, visitor);
	VisitField("initial", v.initial, visitor);
	VisitField("state", v.state, visitor);
}

void Visit(enemyType &v, IDataVisitor *visitor)
{
	VisitField("topLeft", v.topLeft, visitor);
	VisitField("length", v.length, visitor);
	VisitField("delay", v.delay, visitor);
	VisitField("byte0", v.byte0, visitor);
	VisitField("initial", v.initial, visitor);
	VisitField("state", v.state, visitor);
}

void Visit(clutterType &v, IDataVisitor *visitor)
{
	VisitField("bounds", v.bounds, visitor);
	VisitField("pict", v.pict, visitor);
}

void Visit(objectType &v, IDataVisitor *visitor)
{
	IDataVisitor *whatVisitor = visitor->EnterNamed("what");
	if (whatVisitor)
	{
		int ev = v.what;
		if (!VisitEnum(g_objectTypeEnum, g_objectTypeEnumSize, ev, visitor))
			Visit(v.what, visitor);
		whatVisitor->Exit();
	}

	switch (static_cast<int16_t>(v.what))
	{
	default:
		//VisitField("pad", v.data.pad, visitor);
		break;

	case kFloorVent:
	case kCeilingVent:
	case kFloorBlower:
	case kCeilingBlower:
	case kSewerGrate:
	case kLeftFan:
	case kRightFan:
	case kTaper:
	case kCandle:
	case kStubby:
	case kTiki:
	case kBBQ:
	case kInvisBlower:
	case kGrecoVent:
	case kSewerBlower:
	case kLiftArea:
		Visit(v.data.a, visitor);
		break;

	case kTable:
	case kShelf:
	case kCabinet:
	case kFilingCabinet:
	case kWasteBasket:
	case kMilkCrate:
	case kCounter:
	case kDresser:
	case kDeckTable:
	case kStool:
	case kTrunk:
	case kInvisObstacle:
	case kManhole:
	case kBooks:
	case kInvisBounce:
		Visit(v.data.b, visitor);
		break;

	case kRedClock:
	case kBlueClock:
	case kYellowClock:
	case kCuckoo:
	case kPaper:
	case kBattery:
	case kBands:
	case kGreaseRt:
	case kGreaseLf:
	case kFoil:
	case kInvisBonus:
	case kStar:
	case kSparkle:
	case kHelium:
	case kSlider:
		Visit(v.data.c, visitor);
		break;

	case kUpStairs:
	case kDownStairs:
	case kMailboxLf:
	case kMailboxRt:
	case kFloorTrans:
	case kCeilingTrans:
	case kDoorInLf:
	case kDoorInRt:
	case kDoorExRt:
	case kDoorExLf:
	case kWindowInLf:
	case kWindowInRt:
	case kWindowExRt:
	case kWindowExLf:
	case kInvisTrans:
	case kDeluxeTrans:
		Visit(v.data.d, visitor);
		break;

	case kLightSwitch:
	case kMachineSwitch:
	case kThermostat:
	case kPowerSwitch:
	case kKnifeSwitch:
	case kInvisSwitch:
	case kTrigger:
	case kLgTrigger:
	case kSoundTrigger:
		Visit(v.data.e, visitor);
		break;

	case kCeilingLight:
	case kLightBulb:
	case kTableLamp:
	case kHipLamp:
	case kDecoLamp:
	case kFlourescent:
	case kTrackLight:
	case kInvisLight:
		Visit(v.data.f, visitor);
		break;

	case kShredder:
	case kToaster:
	case kMacPlus:
	case kGuitar:
	case kTV:
	case kCoffee:
	case kOutlet:
	case kVCR:
	case kStereo:
	case kMicrowave:
	case kCinderBlock:
	case kFlowerBox:
	case kCDs:
	case kCustomPict:
		Visit(v.data.g, visitor);
		break;

	case kBalloon:
	case kCopterLf:
	case kCopterRt:
	case kDartLf:
	case kDartRt:
	case kBall:
	case kDrip:
	case kFish:
	case kCobweb:
		Visit(v.data.h, visitor);
		break;

	case kOzma:
	case kMirror:
	case kMousehole:
	case kFireplace:
	case kFlower:
	case kWallWindow:
	case kBear:
	case kCalendar:
	case kVase1:
	case kVase2:
	case kBulletin:
	case kCloud:
	case kFaucet:
	case kRug:
	case kChimes:
		Visit(v.data.i, visitor);
		break;
	}
}

void Visit(roomType &v, IDataVisitor *visitor)
{
	VisitField("name", v.name, visitor);
	VisitField("bounds", v.bounds, visitor);
	VisitField("leftStart", v.leftStart, visitor);
	VisitField("rightStart", v.rightStart, visitor);
	VisitField("unusedByte", v.unusedByte, visitor);
	VisitField("visited", v.visited, visitor);
	VisitField("background", v.background, visitor);
	VisitField("tiles", v.tiles, visitor);
	VisitField("floor", v.floor, visitor);
	VisitField("suite", v.suite, visitor);
	VisitField("openings", v.openings, visitor);
	VisitField("numObjects", v.numObjects, visitor);
	VisitField("objects", v.objects, visitor);
}

void Visit(scoresType &v, IDataVisitor *visitor)
{
	VisitField("banner", v.banner, visitor);
	VisitField("names", v.names, visitor);
	VisitField("scores", v.scores, visitor);
	VisitField("timeStamps", v.timeStamps, visitor);
	VisitField("levels", v.levels, visitor);
}

void Visit(gameType &v, IDataVisitor *visitor)
{
	VisitField("version", v.version, visitor);
	VisitField("starsLeft", v.wasStarsLeft, visitor);
	VisitField("timeStamp", v.timeStamp, visitor);
	VisitField("where", v.where, visitor);
	VisitField("score", v.score, visitor);
	VisitField("unusedLong", v.unusedLong, visitor);
	VisitField("unusedLong2", v.unusedLong2, visitor);
	VisitField("energy", v.energy, visitor);
	VisitField("bands", v.bands, visitor);
	VisitField("roomNumber", v.roomNumber, visitor);
	VisitField("gliderState", v.gliderState, visitor);
	VisitField("numGliders", v.numGliders, visitor);
	VisitField("foil", v.foil, visitor);
	VisitField("unusedShort", v.unusedShort, visitor);
	VisitField("facing", v.facing, visitor);
	VisitField("showFoil", v.showFoil, visitor);
}

void Visit(Bool8 &v, IDataVisitor *visitor)
{
	visitor->VisitBool8(v.m_value);
}

void Visit(uint8_t &v, IDataVisitor *visitor)
{
	visitor->VisitByte(v);
}

void Visit(BEInt16_t &v, IDataVisitor *visitor)
{
	int16_t vi = v;
	visitor->VisitInt16(vi);
	v = vi;
}

void Visit(BEInt32_t &v, IDataVisitor *visitor)
{
	int32_t vi = v;
	visitor->VisitInt32(vi);
	v = vi;
}

void Visit(BEUInt32_t &v, IDataVisitor *visitor)
{
	uint32_t vi = v;
	visitor->VisitUInt32(vi);
	v = vi;
}

void Visit(BEPoint &v, IDataVisitor *visitor)
{
	VisitField("h", v.h, visitor);
	VisitField("v", v.v, visitor);
}

void Visit(BERect &v, IDataVisitor *visitor)
{
	VisitField("top", v.top, visitor);
	VisitField("left", v.left, visitor);
	VisitField("bottom", v.bottom, visitor);
	VisitField("right", v.right, visitor);
}

int PrintUsage()
{
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "HouseTool decompile <house.gpd> <out.txt>\n");
	fprintf(stderr, "HouseTool patch <house.gpd> <data def> <replacement value>\n");
	return -1;
}

bool ReadHouseFromPath(const char *path, houseType &house, std::vector<roomType> &rooms)
{
	FILE *f = fopen_utf8(path, "rb");
	if (!f)
		return false;

	if (fread(&house, 1, houseType::kBinaryDataSize, f) != houseType::kBinaryDataSize)
	{
		fclose(f);
		return false;
	}

	int16_t nRooms = house.nRooms;
	if (nRooms < 0)
	{
		fclose(f);
		return false;
	}

	rooms.resize(nRooms);

	if (nRooms > 0)
	{
		if (fread(&rooms[0], 1, sizeof(roomType) * nRooms, f) != sizeof(roomType) * nRooms)
		{
			rooms.clear();
			fclose(f);
			return false;
		}
	}

	fclose(f);
	return true;
}


class PatchVisitor final : public IDataVisitor
{
public:
	PatchVisitor(const char *scope, const char *replacementValue);

	IDataVisitor *EnterIndex(int index) override;
	IDataVisitor *EnterNamed(const char *name) override;
	void Exit() override;

	void VisitLPStr(uint8_t &length, uint8_t *chars, int capacity) override;
	void VisitInt32(int32_t &v) override;
	void VisitUInt32(uint32_t &v) override;
	void VisitInt16(int16_t &v) override;
	void VisitBool8(uint8_t &v) override;
	void VisitByte(uint8_t &v) override;
	bool VisitEnum(const EnumDef *enumDefs, size_t numDefs, int &v) override;

	bool VisitStaticCapacity(size_t size) override;
	void VisitDynamicCapacity(size_t &size) override;

	bool PatchedOK() const;

private:
	static bool DecodeInteger(const std::string &scopeStr, size_t startPos, int &outInteger, size_t &outEndPos);
	static bool DecodeQuotedString(const std::string &scopeStr, size_t startPos, std::string &outStr, size_t &outEndPos);

	std::string m_scope;
	std::string m_nextScope;
	int m_scopeInteger;
	bool m_nextScopeIsInteger;
	const char *m_replacementValue;

	void EnterScope();
	bool TryEnterScope();

	bool m_isDone;
	bool m_haveScope;
	bool m_patchedOK;
};

PatchVisitor::PatchVisitor(const char *scope, const char *replacementValue)
	: m_scopeInteger(0)
	, m_nextScopeIsInteger(false)
	, m_replacementValue(replacementValue)
	, m_nextScope(scope)
	, m_isDone(false)
	, m_haveScope(true)
	, m_patchedOK(false)
{
	EnterScope();
}

IDataVisitor *PatchVisitor::EnterIndex(int index)
{
	if (m_isDone || !m_haveScope || !m_nextScopeIsInteger)
		return nullptr;

	if (m_scopeInteger == index)
	{
		EnterScope();
		return this;
	}

	return nullptr;
}

IDataVisitor *PatchVisitor::EnterNamed(const char *name)
{
	if (m_isDone || !m_haveScope || m_nextScopeIsInteger)
		return nullptr;

	if (m_scope == name)
	{
		EnterScope();
		return this;
	}

	return nullptr;
}

void PatchVisitor::Exit()
{
	m_isDone = true;
}

void PatchVisitor::VisitLPStr(uint8_t &length, uint8_t *chars, int capacity)
{
	if (m_isDone || m_haveScope)
		return;

	const uint8_t *replacementUTF8 = reinterpret_cast<const uint8_t *>(m_replacementValue);
	size_t rLen = strlen(m_replacementValue);

	length = 0;
	while (rLen > 0)
	{
		uint32_t codePoint = 0;
		size_t charsDigested = 0;
		if (!GpUnicode::UTF8::Decode(replacementUTF8, rLen, charsDigested, codePoint))
			break;

		rLen -= charsDigested;
		replacementUTF8 += charsDigested;

		uint8_t macRomanChar = 0;
		if (codePoint > 0xffff)
			macRomanChar = '?';
		else
		{
			if (!MacRoman::FromUnicode(macRomanChar, static_cast<uint16_t>(codePoint)))
				macRomanChar = '?';
		}

		chars[length++] = macRomanChar;
		if (length == capacity)
			break;
	}

	for (int i = length; i < capacity; i++)
		chars[i] = 0;

	m_patchedOK = true;
	m_isDone = true;
}

void PatchVisitor::VisitInt32(int32_t &v)
{
	if (m_isDone || m_haveScope)
		return;

	int integer = 0;
	if (sscanf(m_replacementValue, "%i", &integer) == 1)
	{
		m_patchedOK = true;
		m_isDone = true;
		v = integer;
	}
}

void PatchVisitor::VisitUInt32(uint32_t &v)
{
	if (m_isDone || m_haveScope)
		return;

	unsigned int integer = 0;
	if (sscanf(m_replacementValue, "%u", &integer) == 1)
	{
		m_patchedOK = true;
		m_isDone = true;
		v = integer;
	}
}

void PatchVisitor::VisitInt16(int16_t &v)
{
	if (m_isDone || m_haveScope)
		return;

	int integer = 0;
	if (sscanf(m_replacementValue, "%i", &integer) == 1)
	{
		m_patchedOK = true;
		m_isDone = true;
		v = integer;
	}
}

void PatchVisitor::VisitBool8(uint8_t &v)
{
	if (m_isDone || m_haveScope)
		return;

	if (!strcmp(m_replacementValue, "true"))
	{
		m_patchedOK = true;
		m_isDone = true;
		v = 1;
	}
	else if (!strcmp(m_replacementValue, "false"))
	{
		m_patchedOK = true;
		m_isDone = true;
		v = 0;
	}
	else
	{
		int integer = 0;
		if (sscanf(m_replacementValue, "%i", &integer) == 1)
		{
			m_patchedOK = true;
			m_isDone = true;
			v = integer;
		}
	}
}

void PatchVisitor::VisitByte(uint8_t &v)
{
	if (m_isDone || m_haveScope)
		return;

	int integer = 0;
	if (sscanf(m_replacementValue, "%i", &integer) == 1)
	{
		m_patchedOK = true;
		m_isDone = true;
		v = integer;
	}
}

bool PatchVisitor::VisitEnum(const EnumDef *enumDefs, size_t numDefs, int &v)
{
	if (m_isDone || m_haveScope)
		return false;

	for (size_t i = 0; i < numDefs; i++)
	{
		if (!strcmp(enumDefs[i].m_name, m_replacementValue))
		{
			m_patchedOK = true;
			m_isDone = true;
			v = enumDefs[i].m_value;
			return true;
		}
	}

	return false;
}

bool PatchVisitor::VisitStaticCapacity(size_t size)
{
	return true;
}

void PatchVisitor::VisitDynamicCapacity(size_t &size)
{
}

bool PatchVisitor::PatchedOK() const
{
	return m_patchedOK;
}

bool PatchVisitor::DecodeInteger(const std::string &scopeStr, size_t startPos, int &outInteger, size_t &outEndPos)
{
	size_t len = scopeStr.length();

	if (startPos >= len)
		return false;

	bool isNegative = false;
	char firstChar = scopeStr[startPos];
	if (firstChar == '-')
	{
		isNegative = true;
		startPos++;

		if (startPos >= len)
			return false;
	}

	int resultInteger = 0;

	for (size_t i = startPos; i < len; i++)
	{
		char decChar = scopeStr[i];
		if (decChar >= '0' && decChar <= '9')
		{
			int digit = decChar - '0';
			if (isNegative)
				resultInteger = resultInteger * 10 - digit;
			else
				resultInteger = resultInteger * 10 + digit;
		}
		else
		{
			if (i == startPos)
				return false;

			outInteger = resultInteger;
			outEndPos = i;
			return true;
		}
	}

	outInteger = resultInteger;
	outEndPos = len;
	return true;
}

bool PatchVisitor::DecodeQuotedString(const std::string &scopeStr, size_t startPos, std::string &outString, size_t &outEndPos)
{
	size_t len = scopeStr.length();

	if (startPos >= len || scopeStr[startPos] != '\"')
		return false;

	std::vector<char> decoded;
	for (size_t i = startPos + 1; i < len; i++)
	{
		char decChar = scopeStr[i];
		if (decChar == '\"')
		{
			if (decoded.size() == 0)
				outString = std::string();
			else
				outString = std::string(&decoded[0], decoded.size());

			outEndPos = i + 1;
			return true;
		}

		if (decChar == '\\')
		{
			i++;
			if (i == len)
				return false;

			char escChar = scopeStr[i];
			if (escChar == '\"')
				decoded.push_back('\"');
			else if (escChar == '\\')
				decoded.push_back('\\');
			else if (escChar == '/')
				decoded.push_back('/');
			else if (escChar == 'b')
				decoded.push_back('\b');
			else if (escChar == 'f')
				decoded.push_back('\f');
			else if (escChar == 'n')
				decoded.push_back('\n');
			else if (escChar == 'r')
				decoded.push_back('\r');
			else if (escChar == 't')
				decoded.push_back('\t');
			else if (escChar == 'u')
			{
				int nibbles[4];
				for (int ni = 0; ni < 4; ni++)
				{
					i++;
					if (i == len)
						return false;

					char nibbleChar = scopeStr[i];
					if (nibbleChar >= '0' && nibbleChar <= '9')
						nibbles[ni] = nibbleChar - '0';
					else if (nibbleChar >= 'A' && nibbleChar <= 'F')
						nibbles[ni] = nibbleChar - 'A' + 0xa;
					else if (nibbleChar >= 'a' && nibbleChar <= 'f')
						nibbles[ni] = nibbleChar - 'a' + 0xa;
					else
						return false;
				}

				const uint16_t unicodeCodePoint = (nibbles[0] << 12) + (nibbles[1] << 8) + (nibbles[2] << 4) + nibbles[3];
				uint8_t encoded[GpUnicode::UTF8::kMaxEncodedBytes];
				size_t emitted = 0;
				GpUnicode::UTF8::Encode(encoded, emitted, unicodeCodePoint);

				for (size_t ei = 0; ei < emitted; ei++)
					decoded.push_back(static_cast<char>(encoded[ei]));
			}
		}
		else
		{
			if (decChar < ' ' || decChar > '~')
				return false;

			decoded.push_back(decChar);
		}
	}

	outEndPos = len;
	return false;
}

void PatchVisitor::EnterScope()
{
	if (!m_haveScope)
	{
		m_isDone = true;
		return;
	}

	bool shouldHaveScope = (m_nextScope.size() > 0);
	if (!shouldHaveScope)
	{
		m_haveScope = false;
		return;
	}

	if (!TryEnterScope())
	{
		m_haveScope = false;
		m_isDone = true;
	}
}

bool PatchVisitor::TryEnterScope()
{
	if (m_nextScope.size() == 0)
		return false;

	if (m_nextScope[0] == '.')
	{
		size_t lastFieldChar = 1;
		while (lastFieldChar < m_nextScope.size())
		{
			char c = m_nextScope[lastFieldChar];
			if ((c >= 'a' && c <= 'z')
				|| (c >= 'A' && c <= 'Z')
				|| (c >= '0' && c <= '9')
				|| c == '_')
				lastFieldChar++;
			else
				break;
		}

		if (lastFieldChar == 1)
			return false;

		m_nextScopeIsInteger = false;
		m_scope = m_nextScope.substr(1, lastFieldChar - 1);
		m_nextScope = m_nextScope.substr(lastFieldChar);

		return true;
	}

	if (m_nextScope[0] == '[')
	{
		if (m_nextScope.size() == 1)
			return false;

		if (m_nextScope[1] == '\"')
		{
			std::string quotedStr;
			size_t endPos = 0;
			if (!DecodeQuotedString(m_nextScope, 1, quotedStr, endPos))
				return false;

			if (endPos >= m_nextScope.size())
				return false;

			if (m_nextScope[endPos] != ']')
				return false;

			m_nextScopeIsInteger = false;
			m_scope = quotedStr;
			m_nextScope = m_nextScope.substr(endPos + 1);

			return true;
		}
		else
		{
			int integer = 0;
			size_t endPos = 0;
			if (!DecodeInteger(m_nextScope, 1, integer, endPos))
				return false;

			if (endPos >= m_nextScope.size())
				return false;

			if (m_nextScope[endPos] != ']')
				return false;

			m_nextScopeIsInteger = true;
			m_scope = "";
			m_scopeInteger = integer;
			m_nextScope = m_nextScope.substr(endPos + 1);

			return true;
		}
	}

	return false;
}


int PatchMain(int argc, const char **argv)
{
	if (argc != 5)
		return PrintUsage();

	houseType house;
	std::vector<roomType> rooms;

	if (!ReadHouseFromPath(argv[2], house, rooms))
	{
		fprintf(stderr, "Failed to read house\n");
		return -1;
	}

	PatchVisitor patcher(argv[3], argv[4]);
	Visit(house, rooms, &patcher);

	if (!patcher.PatchedOK())
		fprintf(stderr, "Patch failed\n");
	else
	{
		FILE *houseOutF = fopen_utf8(argv[2], "wb");
		if (!houseOutF)
		{
			fprintf(stderr, "Failed to open output house\n");
			return -1;
		}

		fwrite(&house, 1, houseType::kBinaryDataSize, houseOutF);
		if (rooms.size() > 0)
			fwrite(&rooms[0], 1, sizeof(roomType) * rooms.size(), houseOutF);

		fclose(houseOutF);
	}

	return 0;
}

class DecompileVisitor final : public IDataVisitor
{
public:
	explicit DecompileVisitor(FILE *f);

	IDataVisitor *EnterIndex(int index) override;
	IDataVisitor *EnterNamed(const char *name) override;
	void Exit() override;

	void VisitLPStr(uint8_t &length, uint8_t *chars, int capacity) override;
	void VisitInt32(int32_t &v) override;
	void VisitUInt32(uint32_t &v) override;
	void VisitInt16(int16_t &v) override;
	void VisitBool8(uint8_t &v) override;
	void VisitByte(uint8_t &v) override;
	bool VisitEnum(const EnumDef *enumDefs, size_t numDefs, int &v) override;

	bool VisitStaticCapacity(size_t size) override;
	void VisitDynamicCapacity(size_t &size) override;

private:
	enum ScopeType
	{
		ScopeType_Array,
		ScopeType_Map,
		ScopeType_Unknown,
	};

	FILE *m_f;
	int m_indentLevel;

	bool m_isInNamed;
	bool m_isInArray;

	std::vector<ScopeType> m_scopes;

	ScopeType GetTopScope();
	void SetTopScope(ScopeType scopeType);
	void PopScope();
	void PushScope();

	void WriteIndent();
};

DecompileVisitor::DecompileVisitor(FILE *f)
	: m_f(f)
{
	m_scopes.push_back(ScopeType_Unknown);
}

IDataVisitor *DecompileVisitor::EnterIndex(int index)
{
	ScopeType topScope = GetTopScope();
	if (topScope == ScopeType_Array)
	{
		fprintf(m_f, ",\n");
		WriteIndent();
	}
	else
	{
		fprintf(m_f, "[\n");
		WriteIndent();
		SetTopScope(ScopeType_Array);
	}

	PushScope();

	fprintf(m_f, "[%i] = ", index);

	return this;
}

IDataVisitor *DecompileVisitor::EnterNamed(const char *name)
{
	ScopeType topScope = GetTopScope();
	if (topScope == ScopeType_Map)
	{
		fprintf(m_f, ",\n");
		WriteIndent();
	}
	else
	{
		fprintf(m_f, "{\n");
		WriteIndent();
		SetTopScope(ScopeType_Map);
	}

	PushScope();

	fprintf(m_f, "%s = ", name);

	return this;
}

void DecompileVisitor::Exit()
{
	ScopeType topScope = GetTopScope();
	if (topScope == ScopeType_Map)
	{
		fprintf(m_f, "\n");
		PopScope();
		WriteIndent();
		fprintf(m_f, "}");
	}
	else if (topScope == ScopeType_Array)
	{
		fprintf(m_f, "\n");
		PopScope();
		WriteIndent();
		fprintf(m_f, "]");
	}
	else
		PopScope();
}


void DecompileVisitor::VisitLPStr(uint8_t &length, uint8_t *chars, int capacity)
{
	fputc('\"', m_f);
	for (size_t i = 0; i < length; i++)
	{
		uint16_t unicodeChar = MacRoman::ToUnicode(chars[i]);
		if (unicodeChar == '\\')
			fputs("\\\\", m_f);
		else if (unicodeChar == '\"')
			fputs("\\\"", m_f);
		else if (unicodeChar == '\r')
			fputs("\\r", m_f);
		else if (unicodeChar == '\n')
			fputs("\\n", m_f);
		else if (unicodeChar < 32 || unicodeChar > 126)
			fprintf(m_f, "\\u%04x", unicodeChar);
		else
		{
			uint8_t ubyte = static_cast<uint8_t>(unicodeChar);
			fwrite(&ubyte, 1, 1, m_f);
		}
	}
	fputc('\"', m_f);
}

void DecompileVisitor::VisitInt32(int32_t &v)
{
	fprintf(m_f, "%i", static_cast<int>(v));
}

void DecompileVisitor::VisitUInt32(uint32_t &v)
{
	fprintf(m_f, "%u", static_cast<unsigned int>(v));
}


void DecompileVisitor::VisitInt16(int16_t &v)
{
	fprintf(m_f, "%i", static_cast<int>(v));
}

void DecompileVisitor::VisitBool8(uint8_t &v)
{
	if (v == 0)
		fprintf(m_f, "false");
	else if (v == 1)
		fprintf(m_f, "true");
	else
		fprintf(m_f, "%i", static_cast<int>(v));
}

void DecompileVisitor::VisitByte(uint8_t &v)
{
	fprintf(m_f, "%i", static_cast<int>(v));
}

bool DecompileVisitor::VisitEnum(const EnumDef *enumDefs, size_t numDefs, int &v)
{
	for (size_t i = 0; i < numDefs; i++)
	{
		if (enumDefs[i].m_value == v)
		{
			fprintf(m_f, "\"%s\"", enumDefs[i].m_name);
			return true;
		}
	}

	return false;
}

bool DecompileVisitor::VisitStaticCapacity(size_t size)
{
	return true;
}

void DecompileVisitor::VisitDynamicCapacity(size_t &size)
{
}

DecompileVisitor::ScopeType DecompileVisitor::GetTopScope()
{
	return m_scopes[m_scopes.size() - 1];
}

void DecompileVisitor::SetTopScope(ScopeType scopeType)
{
	m_scopes[m_scopes.size() - 1] = scopeType;
}

void DecompileVisitor::PopScope()
{
	m_scopes.pop_back();
}

void DecompileVisitor::PushScope()
{
	m_scopes.push_back(ScopeType_Unknown);
}

void DecompileVisitor::WriteIndent()
{
	for (size_t i = 0; i < m_scopes.size(); i++)
		fputc('\t', m_f);
}

int DecompileMain(int argc, const char **argv)
{
	if (argc != 4)
		return PrintUsage();

	houseType house;
	std::vector<roomType> rooms;

	if (!ReadHouseFromPath(argv[2], house, rooms))
	{
		fprintf(stderr, "Failed to read house\n");
		return -1;
	}

	FILE *outF = fopen_utf8(argv[3], "wb");
	if (!outF)
	{
		fprintf(stderr, "Failed to open output house\n");
		return -1;
	}

	DecompileVisitor decompiler(outF);
	Visit(house, rooms, &decompiler);

	fclose(outF);

	return 0;
}

int toolMain(int argc, const char **argv)
{
	static const size_t houseSize = sizeof(houseType);
	static_assert(sizeof(houseType) >= houseType::kBinaryDataSize, "House type is too small");

	if (argc < 3)
		return PrintUsage();

	std::string operation = argv[1];

	if (operation == "patch")
		return PatchMain(argc, argv);

	if (operation == "decompile")
		return DecompileMain(argc, argv);

	return 0;
}
