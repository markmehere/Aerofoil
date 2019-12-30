
//============================================================================
//----------------------------------------------------------------------------
//								  AnimCursor.c
//----------------------------------------------------------------------------
//============================================================================


#include "PLResources.h"
#include "PLBigEndian.h"
#include "Externs.h"
#include "Environ.h"


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
		THandle<CCursor> cursorHdl;
	} frame[1];
} compiledAcurRec;

typedef THandle<compiledAcurRec> compiledAcurHandle;


Boolean GetColorCursors (acurHandle, compiledAcurHandle);
void InitAnimatedCursor (acurHandle);


acurHandle			animCursorH = nil;
compiledAcurHandle	compiledAnimCursorH = nil;
Boolean				useColorCursor = false;


//==============================================================  Functions

//--------------------------------------------------------------  GetColorCursors

// Loads color cursors (for animated beach ball).

Boolean GetColorCursors (acurHandle ballCursH, compiledAcurHandle compiledBallCursH)
{
	short			i, j;
	CCrsrHandle		cursHdl;
	Boolean			result = true;
	
	if (ballCursH)
	{
		j = (*ballCursH)->n;				// Get the number of cursors
		HideCursor();						// Hide the cursor
		for (i = 0; i < j; i++)				// Walk through the acur resource
		{
			cursHdl = GetCCursor((*ballCursH)->frame[i].resID);	// Get the cursor
			if (cursHdl == nil)		// Make sure a real cursor was returned
			{								// If not, trash all cursors loaded
				for (j = 0; j < i; j++)				
					DisposeCCursor((CCrsrHandle)(*compiledBallCursH)->frame[j].cursorHdl);
				result = false;				// Tell calling proc we failed
				break;						// And break out of the loop
			}
			else							// But, if the cursor loaded ok
			{								// add it to our list or cursor handles
				(*compiledBallCursH)->frame[i].cursorHdl = cursHdl;
				SetCCursor((CCrsrHandle)(*compiledBallCursH)->frame[i].cursorHdl);
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
	Boolean				useColor;
	compiledAcurHandle	compiledBallCursorH;
	
	useColor = thisMac.hasColor;
	if (ballCursH == nil) 
		ballCursH = GetResource('acur', 128).StaticCast<acurRec>();
	if (ballCursH && ballCursH != animCursorH)
	{
		compiledBallCursorH = NewHandle(sizeof(compiledAcurRec) * (*ballCursH)->n).StaticCast<compiledAcurRec>();
		if (!compiledBallCursorH)
			RedAlert(kErrFailedResourceLoad);

		if (useColor)
			useColor = GetColorCursors(ballCursH, compiledBallCursorH);
		DisposCursors();

		animCursorH = ballCursH;
		compiledAnimCursorH = compiledBallCursorH;
		useColorCursor = useColor;
		(*ballCursH)->index = 0;
	}
	else
		RedAlert(kErrFailedResourceLoad);
}

//--------------------------------------------------------------  LoadCursors

// Just calls the above function.  Other code could be added here though…
// to add additional cursors.

void LoadCursors (void)
{
	InitAnimatedCursor(GetResource('acur', rAcurID).StaticCast<acurRec>());
}

//--------------------------------------------------------------  DisposCursors

// Disposes of all memory allocated by anaimated beach ball cursors.

void DisposCursors (void)
{
	register short		i, j;
	
	if (compiledAnimCursorH != nil)
	{
		j = (*animCursorH)->n;
		if (useColorCursor)
		{
			for (i = 0; i < j; i++)
			{
				if ((*compiledAnimCursorH)->frame[i].cursorHdl != nil)
					DisposeCCursor((CCrsrHandle)(*compiledAnimCursorH)->frame[i].cursorHdl);
			}
		}
		else
		{
			for (i = 0; i < j; i++)
			{
				if ((*compiledAnimCursorH)->frame[i].cursorHdl != nil)
					(*compiledAnimCursorH)->frame[i].cursorHdl.Dispose();
			}
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
		if (useColorCursor)
		{
			SetCCursor((CCrsrHandle)(*compiledAnimCursorH)->
					frame[(*animCursorH)->index].cursorHdl);
		}
		else
		{
			SetCursor((CursPtr)*(*compiledAnimCursorH)->
					frame[(*animCursorH)->index].cursorHdl);
		}
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
		if (useColorCursor)
		{
			SetCCursor((CCrsrHandle)(*compiledAnimCursorH)->
					frame[(*animCursorH)->index].cursorHdl);
		}
		else
		{
			SetCursor((CursPtr)*(*compiledAnimCursorH)->
					frame[(*animCursorH)->index].cursorHdl);
		}
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

