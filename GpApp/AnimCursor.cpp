
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
} acurRec, *acurPtr, **acurHandle;

typedef struct
{
	struct
	{
		Handle cursorHdl;
	} frame[1];
} compiledAcurRec, *compiledAcurPtr, **compiledAcurHandle;


Boolean GetMonoCursors (acurHandle, compiledAcurHandle);
Boolean GetColorCursors (acurHandle, compiledAcurHandle);
void InitAnimatedCursor (acurHandle);


acurHandle			animCursorH = nil;
compiledAcurHandle	compiledAnimCursorH = nil;
Boolean				useColorCursor = false;


//==============================================================  Functions
//--------------------------------------------------------------  GetMonoCursors

// Loads b&w cursors (for animated beach ball).

Boolean GetMonoCursors (acurHandle ballCursH, compiledAcurHandle compiledBallCursH)
{
	short			i, j;
	CursHandle		cursHdl;
	
	if (ballCursH)							// Were we passed a legit acur handle?
	{
		j = (*ballCursH)->n;				// Get number of 'frames' in the acur
		for (i = 0; i < j; i++)				// Start walking the frames
		{
			cursHdl = GetCursor((*ballCursH)->frame[i].resID);
			if (cursHdl == nil)		// Did the cursor load? It didn't?...
			{								// Well then, toss what we got.
				for (j = 0; j < i; j++)
					DisposeHandle((*compiledBallCursH)->frame[j].cursorHdl);
				return(false);				// And report this to mother.
			}								// However!...
			else							// If cursor loaded ok...
			{								// Detach it from the resource map...
				DetachResource((Handle)cursHdl);	// And assign to our struct
				(*compiledBallCursH)->frame[i].cursorHdl = (Handle)cursHdl;
			}
		}
	}
	return(true);
}

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
				(*compiledBallCursH)->frame[i].cursorHdl = (Handle)cursHdl;
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
		ballCursH = reinterpret_cast<acurHandle>(GetResource('acur', 128));
	if (ballCursH && ballCursH != animCursorH)
	{
		compiledBallCursorH = (compiledAcurHandle)NewHandle(sizeof(compiledAcurRec) * (*ballCursH)->n);
		if (!compiledBallCursorH)
			RedAlert(kErrFailedResourceLoad);

		if (useColor)
			useColor = GetColorCursors(ballCursH, compiledBallCursorH);
		if (!useColor && !GetMonoCursors(ballCursH, compiledBallCursorH))
			RedAlert(kErrFailedResourceLoad);
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

// Just calls the above function.  Other code could be added here thoughÉ
// to add additional cursors.

void LoadCursors (void)
{
	InitAnimatedCursor((acurHandle)GetResource('acur', rAcurID));
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
					DisposeHandle((Handle)(*compiledAnimCursorH)->frame[i].cursorHdl);
			}
		}
		DisposeHandle((Handle)compiledAnimCursorH);
		compiledAnimCursorH = nil;
	}

	if (animCursorH != nil)
	{
		ReleaseResource((Handle)animCursorH);
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

