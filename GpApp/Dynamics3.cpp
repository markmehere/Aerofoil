
//============================================================================
//----------------------------------------------------------------------------
//								   Dynamics3.c
//----------------------------------------------------------------------------
//============================================================================


#include "Externs.h"
#include "RectUtils.h"


#define kBalloonStart		310
#define kCopterStart		8
#define kDartVelocity		6


dynaPtr		dinahs;
short		numDynamics;

extern	Rect		breadSrc[];
extern	short		numLights;
extern	Boolean		evenFrame;


//==============================================================  Functions
//--------------------------------------------------------------  HandleDynamics

// This is the master function that calls all the specific handlers above.

void HandleDynamics (void)
{
	short		i;
	
	for (i = 0; i < numDynamics; i++)
	{
		switch (dinahs[i].type)
		{
			case kSparkle:
			HandleSparkleObject(i);
			break;
			
			case kToaster:
			HandleToast(i);
			break;
			
			case kMacPlus:
			HandleMacPlus(i);
			break;
			
			case kTV:
			HandleTV(i);
			break;
			
			case kCoffee:
			HandleCoffee(i);
			break;
			
			case kOutlet:
			HandleOutlet(i);
			break;
			
			case kVCR:
			HandleVCR(i);
			break;
			
			case kStereo:
			HandleStereo(i);
			break;
			
			case kMicrowave:
			HandleMicrowave(i);
			break;
			
			case kBalloon:
			HandleBalloon(i);
			break;
			
			case kCopterLf:
			case kCopterRt:
			HandleCopter(i);
			break;
			
			case kDartLf:
			case kDartRt:
			HandleDart(i);
			break;
			
			case kBall:
			HandleBall(i);
			break;
			
			case kDrip:
			HandleDrip(i);
			break;
			
			case kFish:
			HandleFish(i);
			break;
			
			default:
			break;
		}
	}
}

//--------------------------------------------------------------  HandleDynamics

// This is the master function that calls all the various rendering handlersÉ
// above.

void RenderDynamics (void)
{
	short		i;
	
	for (i = 0; i < numDynamics; i++)
	{
		switch (dinahs[i].type)
		{
			case kToaster:
			RenderToast(i);
			break;
			
			case kBalloon:
			RenderBalloon(i);
			break;
			
			case kCopterLf:
			case kCopterRt:
			RenderCopter(i);
			break;
			
			case kDartLf:
			case kDartRt:
			RenderDart(i);
			break;
			
			case kBall:
			RenderBall(i);
			break;
			
			case kDrip:
			RenderDrip(i);
			break;
			
			case kFish:
			RenderFish(i);
			break;
			
			default:
			break;
		}
	}
}

//--------------------------------------------------------------  ZeroDinah

void ZeroDinah(dynaType &dinah)
{
	dinah.type = kObjectIsEmpty;
	QSetRect(&dinah.dest, 0, 0, 0, 0);
	QSetRect(&dinah.whole, 0, 0, 0, 0);
	dinah.hVel = 0;
	dinah.vVel = 0;
	dinah.count = 0;
	dinah.frame = 0;
	dinah.timer = 0;
	dinah.position = 0;
	dinah.room = 0;
	dinah.byte0 = 0;
	dinah.active = false;
}

//--------------------------------------------------------------  ZeroDinahsNotInRoom
void ZeroDinahsNotInRoom (SInt16 room)
{
	short		i;
	short		newNumDynamics = 0;

	for (i = 0; i < numDynamics; i++)
	{
		dynaType &dinah = dinahs[i];
		if (dinah.room == room)
		{
			if (newNumDynamics != numDynamics)
				memcpy(&dinahs[newNumDynamics], &dinahs[i], sizeof(dynaType));

			newNumDynamics++;
		}
	}

	for (i = newNumDynamics; i < kMaxDynamicObs; i++)
		ZeroDinah(dinahs[i]);

	numDynamics = newNumDynamics;
}

//--------------------------------------------------------------  ZeroDinahs

// This clears all dynamics - zeros them all out.  Used to initialize them.

void ZeroDinahs (void)
{
	short		i;
	
	for (i = 0; i < kMaxDynamicObs; i++)
		ZeroDinah(dinahs[i]);

	numDynamics = 0;
}

//--------------------------------------------------------------  AddDynamicObject

// When a room is being drawn, various dynamic objects are pointed here.
// This function sets up the structures to handle them.

short AddDynamicObject (short what, Rect *where, objectType *who, 
		short room, short index, Boolean isOn, Boolean keepExisting)
{
	short		position, velocity;
	Boolean		lilFrame;

	short		dynIndex = -1;

	if (!keepExisting)
	{
		if (numDynamics >= kMaxDynamicObs)
			return (-1);

		dynIndex = numDynamics;
		numDynamics++;

		dinahs[dynIndex].type = what;
	}
	else
	{
		for (int i = 0; i < numDynamics; i++)
		{
			if (dinahs[i].type == what && dinahs[i].room == room && dinahs[i].byte0 == index)
			{
				dynIndex = i;
				break;
			}
		}

		if (dynIndex == -1)
			return (-1);
	}
	
	switch (what)
	{
	case kSparkle:
		dinahs[dynIndex].dest = sparkleSrc[0];
		ZeroRectCorner(&dinahs[dynIndex].dest);
		QOffsetRect(&dinahs[dynIndex].dest, where->left, where->top);
		dinahs[dynIndex].whole = dinahs[dynIndex].dest;

		if (!keepExisting)
		{
			dinahs[dynIndex].hVel = 0;
			dinahs[dynIndex].vVel = 0;
			dinahs[dynIndex].count = 0;
			dinahs[dynIndex].frame = 0;
			dinahs[dynIndex].timer = RandomInt(60) + 15;
			dinahs[dynIndex].position = 0;
			dinahs[dynIndex].room = room;
			dinahs[dynIndex].byte0 = (Byte)index;
			dinahs[dynIndex].byte1 = 0;
			dinahs[dynIndex].moving = false;
			dinahs[dynIndex].active = isOn;
		}
		break;
		
	case kToaster:
		{
			short baselineDelta = 0;
			if (keepExisting)
				baselineDelta = dinahs[dynIndex].dest.top - (where->top - 2);

			dinahs[dynIndex].dest = breadSrc[0];
			CenterRectInRect(&dinahs[dynIndex].dest, where);
			VOffsetRect(&dinahs[dynIndex].dest, 
				where->top - dinahs[dynIndex].dest.top + baselineDelta);

			dinahs[dynIndex].whole = dinahs[dynIndex].dest;
			dinahs[dynIndex].hVel = where->top + 2;	// hVel used as clip

			if (!keepExisting)
			{
				position = who->data.g.height;				// reverse engineer init. vel.
				velocity = 0;
				do
				{
					velocity++;
					position -= velocity;
				} while (position > 0);
				dinahs[dynIndex].vVel = -velocity;
				dinahs[dynIndex].count = velocity;		// count = initial velocity
				dinahs[dynIndex].frame = (short)who->data.g.delay * 3;
				dinahs[dynIndex].timer = dinahs[dynIndex].frame;
				dinahs[dynIndex].position = 0;			// launch/idle state
				dinahs[dynIndex].room = room;
				dinahs[dynIndex].byte0 = (Byte)index;
				dinahs[dynIndex].byte1 = 0;
				dinahs[dynIndex].moving = false;
				dinahs[dynIndex].active = isOn;
			}
		}
		break;
		
	case kMacPlus:
		dinahs[dynIndex].dest = plusScreen1;
		ZeroRectCorner(&dinahs[dynIndex].dest);
		QOffsetRect(&dinahs[dynIndex].dest, 
				where->left + playOriginH + 10, 
				where->top + playOriginV + 7);
		dinahs[dynIndex].whole = dinahs[dynIndex].dest;
		if (!keepExisting)
		{
			dinahs[dynIndex].hVel = 0;
			dinahs[dynIndex].vVel = 0;
			dinahs[dynIndex].count = 0;
			dinahs[dynIndex].frame = 0;
			dinahs[dynIndex].timer = 0;
			dinahs[dynIndex].position = 0;
			dinahs[dynIndex].room = room;
			dinahs[dynIndex].byte0 = (Byte)index;
			dinahs[dynIndex].byte1 = 0;
			dinahs[dynIndex].moving = false;
			dinahs[dynIndex].active = isOn;
		}
		break;
		
	case kTV:
		dinahs[dynIndex].dest = tvScreen1;
		ZeroRectCorner(&dinahs[dynIndex].dest);
		QOffsetRect(&dinahs[dynIndex].dest, 
				where->left + playOriginH + 17, 
				where->top + playOriginV + 10);
		dinahs[dynIndex].whole = dinahs[dynIndex].dest;
		if (!keepExisting)
		{
			dinahs[dynIndex].hVel = 0;
			dinahs[dynIndex].vVel = 0;
			dinahs[dynIndex].count = 0;
			dinahs[dynIndex].frame = 0;
			dinahs[dynIndex].timer = 0;
			dinahs[dynIndex].position = 0;
			dinahs[dynIndex].room = room;
			dinahs[dynIndex].byte0 = (Byte)index;
			dinahs[dynIndex].byte1 = 0;
			dinahs[dynIndex].moving = false;
			dinahs[dynIndex].active = isOn;
		}
		break;
		
	case kCoffee:
		dinahs[dynIndex].dest = coffeeLight1;
		ZeroRectCorner(&dinahs[dynIndex].dest);
		QOffsetRect(&dinahs[dynIndex].dest, 
				where->left + playOriginH + 32, 
				where->top + playOriginV + 57);
		dinahs[dynIndex].whole = dinahs[dynIndex].dest;

		if (!keepExisting)
		{
			dinahs[dynIndex].hVel = 0;
			dinahs[dynIndex].vVel = 0;
			dinahs[dynIndex].count = 0;
			dinahs[dynIndex].frame = 0;
			if (isOn)
				dinahs[dynIndex].timer = 200;
			else
				dinahs[dynIndex].timer = 0;
			dinahs[dynIndex].position = 0;
			dinahs[dynIndex].room = room;
			dinahs[dynIndex].byte0 = (Byte)index;
			dinahs[dynIndex].byte1 = 0;
			dinahs[dynIndex].moving = false;
			dinahs[dynIndex].active = isOn;
		}
		break;
		
	case kOutlet:
		dinahs[dynIndex].dest = outletSrc[0];
		ZeroRectCorner(&dinahs[dynIndex].dest);
		QOffsetRect(&dinahs[dynIndex].dest, 
				where->left + playOriginH, 
				where->top + playOriginV);
		dinahs[dynIndex].whole = dinahs[dynIndex].dest;

		if (!keepExisting)
		{
			dinahs[dynIndex].hVel = numLights;
			dinahs[dynIndex].vVel = 0;
			dinahs[dynIndex].count = ((short)who->data.g.delay * 6) / kTicksPerFrame;
			dinahs[dynIndex].frame = 0;
			dinahs[dynIndex].timer = dinahs[dynIndex].count;
			dinahs[dynIndex].position = 0;			// launch/idle state
			dinahs[dynIndex].room = room;
			dinahs[dynIndex].byte0 = (Byte)index;
			dinahs[dynIndex].byte1 = 0;
			dinahs[dynIndex].moving = false;
			dinahs[dynIndex].active = isOn;
		}
		break;
		
	case kVCR:
		dinahs[dynIndex].dest = vcrTime1;
		ZeroRectCorner(&dinahs[dynIndex].dest);
		QOffsetRect(&dinahs[dynIndex].dest, 
				where->left + playOriginH + 64, 
				where->top + playOriginV + 6);
		dinahs[dynIndex].whole = dinahs[dynIndex].dest;

		if (!keepExisting)
		{
			dinahs[dynIndex].hVel = 0;
			dinahs[dynIndex].vVel = 0;
			dinahs[dynIndex].count = 0;
			dinahs[dynIndex].frame = 0;
			if (isOn)
				dinahs[dynIndex].timer = 115;
			else
				dinahs[dynIndex].timer = 0;
			dinahs[dynIndex].position = 0;
			dinahs[dynIndex].room = room;
			dinahs[dynIndex].byte0 = (Byte)index;
			dinahs[dynIndex].byte1 = 0;
			dinahs[dynIndex].moving = false;
			dinahs[dynIndex].active = isOn;
		}
		break;
		
	case kStereo:
		dinahs[dynIndex].dest = stereoLight1;
		ZeroRectCorner(&dinahs[dynIndex].dest);
		QOffsetRect(&dinahs[dynIndex].dest, 
				where->left + playOriginH + 56, 
				where->top + playOriginV + 20);
		dinahs[dynIndex].whole = dinahs[dynIndex].dest;

		if (!keepExisting)
		{
			dinahs[dynIndex].hVel = 0;
			dinahs[dynIndex].vVel = 0;
			dinahs[dynIndex].count = 0;
			dinahs[dynIndex].frame = 0;
			dinahs[dynIndex].timer = 0;
			dinahs[dynIndex].position = 0;
			dinahs[dynIndex].room = room;
			dinahs[dynIndex].byte0 = (Byte)index;
			dinahs[dynIndex].byte1 = 0;
			dinahs[dynIndex].moving = false;
			dinahs[dynIndex].active = isOn;
		}
		break;
		
	case kMicrowave:
		dinahs[dynIndex].dest = microOn;
		ZeroRectCorner(&dinahs[dynIndex].dest);
		QOffsetRect(&dinahs[dynIndex].dest, 
				where->left + playOriginH + 14, 
				where->top + playOriginV + 13);
		dinahs[dynIndex].dest.right = dinahs[dynIndex].dest.left + 48;
		dinahs[dynIndex].whole = dinahs[dynIndex].dest;

		if (!keepExisting)
		{
			dinahs[dynIndex].hVel = 0;
			dinahs[dynIndex].vVel = 0;
			dinahs[dynIndex].count = 0;
			dinahs[dynIndex].frame = 0;
			dinahs[dynIndex].timer = 0;
			dinahs[dynIndex].position = 0;
			dinahs[dynIndex].room = room;
			dinahs[dynIndex].byte0 = (Byte)index;
			dinahs[dynIndex].byte1 = 0;
			dinahs[dynIndex].moving = false;
			dinahs[dynIndex].active = isOn;
		}
		break;
		
	case kBalloon:
		dinahs[dynIndex].dest = balloonSrc[0];
		ZeroRectCorner(&dinahs[dynIndex].dest);
		QOffsetRect(&dinahs[dynIndex].dest, where->left, 0);
		dinahs[dynIndex].dest.bottom = kBalloonStart;
		dinahs[dynIndex].dest.top = dinahs[dynIndex].dest.bottom - 
				RectTall(&balloonSrc[0]);
		dinahs[dynIndex].whole = dinahs[dynIndex].dest;

		if (!keepExisting)
		{
			dinahs[dynIndex].hVel = 0;
			dinahs[dynIndex].vVel = -2;
			dinahs[dynIndex].count = ((short)who->data.h.delay * 6) / kTicksPerFrame;
			dinahs[dynIndex].frame = 0;
			dinahs[dynIndex].timer = dinahs[dynIndex].count;
			dinahs[dynIndex].position = 0;
			dinahs[dynIndex].room = room;
			dinahs[dynIndex].byte0 = (Byte)index;
			dinahs[dynIndex].byte1 = 0;
			dinahs[dynIndex].moving = false;
			dinahs[dynIndex].active = isOn;			// initially idle
		}
		break;
		
	case kCopterLf:
	case kCopterRt:
		dinahs[dynIndex].dest = copterSrc[0];
		ZeroRectCorner(&dinahs[dynIndex].dest);
		QOffsetRect(&dinahs[dynIndex].dest, where->left, 0);
		dinahs[dynIndex].dest.top = kCopterStart;
		dinahs[dynIndex].dest.bottom = dinahs[dynIndex].dest.top + 
				RectTall(&copterSrc[0]);
		dinahs[dynIndex].whole = dinahs[dynIndex].dest;
		dinahs[dynIndex].position = dinahs[dynIndex].dest.left;

		if (!keepExisting)
		{
			if (what == kCopterLf)
				dinahs[dynIndex].hVel = -1;
			else
				dinahs[dynIndex].hVel = 1;
			dinahs[dynIndex].vVel = 2;
			dinahs[dynIndex].count = ((short)who->data.h.delay * 6) / kTicksPerFrame;
			dinahs[dynIndex].frame = 0;
			dinahs[dynIndex].timer = dinahs[dynIndex].count;
			dinahs[dynIndex].room = room;
			dinahs[dynIndex].byte0 = (Byte)index;
			dinahs[dynIndex].byte1 = 0;
			dinahs[dynIndex].moving = false;
			dinahs[dynIndex].active = isOn;			// initially idle
		}
		break;
		
	case kDartLf:
	case kDartRt:
		dinahs[dynIndex].dest = dartSrc[0];
		ZeroRectCorner(&dinahs[dynIndex].dest);
		if (what == kDartLf)
		{
			QOffsetRect(&dinahs[dynIndex].dest, 
					kRoomWide - RectWide(&dartSrc[0]), where->top);
			dinahs[dynIndex].hVel = -kDartVelocity;
			dinahs[dynIndex].frame = 0;
		}
		else
		{
			QOffsetRect(&dinahs[dynIndex].dest, 0, where->top);
			dinahs[dynIndex].hVel = kDartVelocity;
			dinahs[dynIndex].frame = 2;
		}
		dinahs[dynIndex].whole = dinahs[dynIndex].dest;
		dinahs[dynIndex].position = dinahs[dynIndex].dest.top;

		if (!keepExisting)
		{
			dinahs[dynIndex].vVel = 2;
			dinahs[dynIndex].count = ((short)who->data.h.delay * 6) / kTicksPerFrame;
			dinahs[dynIndex].timer = dinahs[dynIndex].count;
			dinahs[dynIndex].byte0 = (Byte)index;
			dinahs[dynIndex].byte1 = 0;
			dinahs[dynIndex].moving = false;
			dinahs[dynIndex].active = isOn;			// initially idle
		}
		break;
		
	case kBall:
		dinahs[dynIndex].dest = ballSrc[0];
		ZeroRectCorner(&dinahs[dynIndex].dest);
		QOffsetRect(&dinahs[dynIndex].dest, 
				where->left, where->top);
		dinahs[dynIndex].whole = dinahs[dynIndex].dest;
		dinahs[dynIndex].position = dinahs[dynIndex].dest.bottom;

		if (!keepExisting)
		{
			dinahs[dynIndex].hVel = 0;
			position = who->data.h.length;			// reverse engineer init. vel.
			velocity = 0;
			evenFrame = true;
			lilFrame = true;
			do
			{
				if (lilFrame)
					velocity++;
				lilFrame = !lilFrame;
				position -= velocity;
			} while (position > 0);
			dinahs[dynIndex].vVel = -velocity;
			dinahs[dynIndex].moving = false;
			dinahs[dynIndex].count = -velocity;	// count = initial velocity
			dinahs[dynIndex].frame = 0;
			dinahs[dynIndex].timer = 0;
			dinahs[dynIndex].room = room;
			dinahs[dynIndex].byte0 = (Byte)index;
			dinahs[dynIndex].byte1 = 0;
			dinahs[dynIndex].active = isOn;
		}
		break;
		
	case kDrip:
		dinahs[dynIndex].dest = dripSrc[0];
		CenterRectInRect(&dinahs[dynIndex].dest, where);
		VOffsetRect(&dinahs[dynIndex].dest, 
				where->top - dinahs[dynIndex].dest.top);
		dinahs[dynIndex].whole = dinahs[dynIndex].dest;
		dinahs[dynIndex].hVel = dinahs[dynIndex].dest.top;	// remember
		dinahs[dynIndex].position = dinahs[dynIndex].dest.top +
			who->data.h.length;

		if (!keepExisting)
		{
			dinahs[dynIndex].vVel = 0;
			dinahs[dynIndex].count = ((short)who->data.h.delay * 6) / kTicksPerFrame;
			dinahs[dynIndex].frame = 3;
			dinahs[dynIndex].timer = dinahs[dynIndex].count;
			dinahs[dynIndex].room = room;
			dinahs[dynIndex].byte0 = (Byte)index;
			dinahs[dynIndex].byte1 = 0;
			dinahs[dynIndex].moving = false;
			dinahs[dynIndex].active = isOn;
		}
		break;
		
	case kFish:
		dinahs[dynIndex].dest = fishSrc[0];
		QOffsetRect(&dinahs[dynIndex].dest, 
				where->left + 10, where->top + 8);
		dinahs[dynIndex].whole = dinahs[dynIndex].dest;
		dinahs[dynIndex].position = dinahs[dynIndex].dest.bottom;

		if (!keepExisting)
		{
			dinahs[dynIndex].hVel = ((short)who->data.h.delay * 6) / kTicksPerFrame;
			position = who->data.g.height;			// reverse engineer init. vel.
			velocity = 0;
			evenFrame = true;
			lilFrame = true;
			do
			{
				if (lilFrame)
					velocity++;
				lilFrame = !lilFrame;
				position -= velocity;
			} while (position > 0);
			dinahs[dynIndex].vVel = -velocity;
			dinahs[dynIndex].count = -velocity;	// count = initial velocity
			dinahs[dynIndex].frame = 0;
			dinahs[dynIndex].timer = dinahs[dynIndex].hVel;
			dinahs[dynIndex].room = room;
			dinahs[dynIndex].byte0 = (Byte)index;
			dinahs[dynIndex].byte1 = 0;
			dinahs[dynIndex].moving = false;
			dinahs[dynIndex].active = isOn;
		}
		break;
		
	default:
		break;
	}

	return dynIndex;
}

void OffsetDynamics(SInt16 h, SInt16 v)
{
	for (int i = 0; i < numDynamics; i++)
	{
		dynaType &dinah = dinahs[i];
		//QOffsetRect(&dinah.dest, h, v);
		//QOffsetRect(&dinah.whole, h, v);
	}
}
