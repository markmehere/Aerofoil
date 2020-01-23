
//============================================================================
//----------------------------------------------------------------------------
//								  AnimCursor.c
//----------------------------------------------------------------------------
//============================================================================


#include "PLResources.h"
#include "PLBigEndian.h"
#include "Externs.h"
#include "Environ.h"
#include "HostDisplayDriver.h"
#include "IGpColorCursor.h"
#include "IGpDisplayDriver.h"
#include "ResourceManager.h"


#define	rAcurID					128
#define rHandCursorID			1000


typedef struct
{
	BEInt16_t		n;
	BEInt16_t		index;
	struct
	{
		BEInt16_t	resID;
		BEInt16_t	reserved;
	} frame[1];
} acurRec;

typedef THandle<acurRec> acurHandle;

typedef struct
{
	struct
	{
		IGpColorCursor *hwCursor;
	} frame[1];
} compiledAcurRec;

typedef THandle<compiledAcurRec> compiledAcurHandle;


Boolean GetColorCursors (acurHandle, compiledAcurHandle);
void InitAnimatedCursor (acurHandle);


acurHandle			animCursorH = nil;
compiledAcurHandle	compiledAnimCursorH = nil;


//==============================================================  Functions

//--------------------------------------------------------------  GetColorCursors

// Loads color cursors (for animated beach ball).

Boolean GetColorCursors (acurHandle ballCursH, compiledAcurHandle compiledBallCursH)
{
	short			i, j;
	IGpColorCursor *hwCursor;
	Boolean			result = true;
	
	if (ballCursH)
	{
		j = (*ballCursH)->n;				// Get the number of cursors
		HideCursor();						// Hide the cursor
		for (i = 0; i < j; i++)				// Walk through the acur resource
		{
			hwCursor = PortabilityLayer::HostDisplayDriver::GetInstance()->LoadColorCursor((*ballCursH)->frame[i].resID);	// Get the cursor
			if (hwCursor == nil)		// Make sure a real cursor was returned
			{								// If not, trash all cursors loaded
				for (j = 0; j < i; j++)
					(*compiledBallCursH)->frame[j].hwCursor->Destroy();
				result = false;				// Tell calling proc we failed
				break;						// And break out of the loop
			}
			else							// But, if the cursor loaded ok
			{								// add it to our list or cursor handles
				(*compiledBallCursH)->frame[i].hwCursor = hwCursor;
				PortabilityLayer::HostDisplayDriver::GetInstance()->SetColorCursor(hwCursor);
			}
		}
		InitCursor();						// Show the cursor again (as arrow)
	}
	return(result);							// Return to calling proc w/ results
}

//--------------------------------------------------------------  InitAnimatedCursor

// Loads and sets up animated beach ball cursor structures.

void InitAnimatedCursor (acurHandle ballCursH)
{
	compiledAcurHandle	compiledBallCursorH;
	
	if (ballCursH == nil) 
		ballCursH = PortabilityLayer::ResourceManager::GetInstance()->GetAppResource('acur', 128).StaticCast<acurRec>();
	if (ballCursH && ballCursH != animCursorH)
	{
		compiledBallCursorH = NewHandle(sizeof(compiledAcurRec) * (*ballCursH)->n).StaticCast<compiledAcurRec>();
		if (!compiledBallCursorH)
			RedAlert(kErrFailedResourceLoad);

		GetColorCursors(ballCursH, compiledBallCursorH);
		DisposCursors();

		animCursorH = ballCursH;
		compiledAnimCursorH = compiledBallCursorH;
		(*ballCursH)->index = 0;
	}
	else
		RedAlert(kErrFailedResourceLoad);
}

//--------------------------------------------------------------  LoadCursors

// Just calls the above function.  Other code could be added here thoughÉ
// to add additional cursors.

void LoadCursors (void)
{
	InitAnimatedCursor(PortabilityLayer::ResourceManager::GetInstance()->GetAppResource('acur', rAcurID).StaticCast<acurRec>());
}

//--------------------------------------------------------------  DisposCursors

// Disposes of all memory allocated by anaimated beach ball cursors.

void DisposCursors (void)
{
	register short		i, j;
	
	if (compiledAnimCursorH != nil)
	{
		j = (*animCursorH)->n;

		for (i = 0; i < j; i++)
		{
			if ((*compiledAnimCursorH)->frame[i].hwCursor != nil)
				(*compiledAnimCursorH)->frame[i].hwCursor->Destroy();
		}

		compiledAnimCursorH.Dispose();
		compiledAnimCursorH = nil;
	}

	if (animCursorH != nil)
	{
		animCursorH.Dispose();
		animCursorH = nil;
	}
}

//--------------------------------------------------------------  IncrementCursor

// Advances the beach ball cursor one frame.

void IncrementCursor (void)
{
	if (animCursorH == 0)
		InitAnimatedCursor(nil);
	if (animCursorH)
	{
		(*animCursorH)->index++;
		(*animCursorH)->index %= (*animCursorH)->n;

		PortabilityLayer::HostDisplayDriver::GetInstance()->SetColorCursor((*compiledAnimCursorH)->frame[(*animCursorH)->index].hwCursor);
	}
	else
		SetBuiltinCursor(watchCursor);
}

//--------------------------------------------------------------  DecrementCursor

// Reverses the beach ball cursor one frame.

void DecrementCursor (void)
{
	if (animCursorH == 0)
		InitAnimatedCursor(nil);
	if (animCursorH)
	{
		(*animCursorH)->index--;
		if (((*animCursorH)->index) < 0)
			(*animCursorH)->index = ((*animCursorH)->n) - 1;

		PortabilityLayer::HostDisplayDriver::GetInstance()->SetColorCursor((*compiledAnimCursorH)->frame[(*animCursorH)->index].hwCursor);
	}
	else
		SetBuiltinCursor(watchCursor);
}

//--------------------------------------------------------------  SpinCursor

// Advances the beach ball cursor the number of frames specified.

void SpinCursor (short incrementIndex)
{
	UInt32		dummyLong;
	short		i;
	
	for (i = 0; i < incrementIndex; i++)
	{
		IncrementCursor();
		Delay(1, &dummyLong);
	}
}

//--------------------------------------------------------------  BackSpinCursor

// Reverses the beach ball cursor the number of frames specified.

void BackSpinCursor (short decrementIndex)
{
	UInt32		dummyLong;
	short		i;
	
	for (i = 0; i < decrementIndex; i++)
	{
		DecrementCursor();
		Delay(1, &dummyLong);
	}
}

