#include "PLMovies.h"

OSErr EnterMovies()
{
	return noErr;
}

UserData GetMovieUserData(Movie movie)
{
	PL_NotYetImplemented();
	return nullptr;
}

int CountUserDataType(UserData userData, UInt32 type)
{
	PL_NotYetImplemented();
	return 0;
}

OSErr RemoveUserData(UserData userData, UInt32 type, int index)
{
	PL_NotYetImplemented();
	return noErr;
}

OSErr AddUserData(UserData userData, Handle data, UInt32 type)
{
	PL_NotYetImplemented();
	return noErr;
}

OSErr OpenMovieFile(const FSSpec *fsSpec, short *outRefNum, int permissions)
{
	PL_NotYetImplemented_TODO("Movies");
	return noErr;
}

OSErr NewMovieFromFile(Movie *movie, short refNum, const short *optResId, StringPtr resName, int flags, Boolean *unused)
{
	PL_NotYetImplemented_TODO("Movies");
	return noErr;
}

OSErr CloseMovieFile(short refNum)
{
	PL_NotYetImplemented();
	return noErr;
}

OSErr GoToBeginningOfMovie(Movie movie)
{
	PL_NotYetImplemented();
	return noErr;
}

OSErr LoadMovieIntoRam(Movie movie, TimeValue time, TimeValue duration, int flags)
{
	PL_NotYetImplemented();
	return noErr;
}

TimeValue GetMovieTime(Movie movie, TimeRecord *outCurrentTime)
{
	PL_NotYetImplemented();
	return 0;
}

TimeValue GetMovieDuration(Movie movie)
{
	PL_NotYetImplemented();
	return 0;
}

OSErr PrerollMovie(Movie movie, TimeValue time, UInt32 rate)
{
	PL_NotYetImplemented();
	return noErr;
}

TimeBase GetMovieTimeBase(Movie movie)
{
	PL_NotYetImplemented();
	return nullptr;
}

OSErr SetTimeBaseFlags(TimeBase timeBase, int flags)
{
	PL_NotYetImplemented();
	return noErr;
}

void SetMovieMasterTimeBase(Movie movie, TimeBase timeBase, void *unused)
{
	PL_NotYetImplemented();
}

void GetMovieBox(Movie movie, Rect *rect)
{
	PL_NotYetImplemented();
}

void StopMovie(Movie movie)
{
	PL_NotYetImplemented();
}

void DisposeMovie(Movie movie)
{
	PL_NotYetImplemented();
}

void SetMovieGWorld(Movie movie, CGrafPtr graf, void *unknown)
{
	PL_NotYetImplemented();
}

void SetMovieActive(Movie movie, Boolean active)
{
	PL_NotYetImplemented();
}

void StartMovie(Movie movie)
{
	PL_NotYetImplemented();
}

void MoviesTask(Movie movie, int unknown)
{
	PL_NotYetImplemented();
}

void SetMovieBox(Movie movie, const Rect *rect)
{
	PL_NotYetImplemented();
}

void SetMovieDisplayClipRgn(Movie movie, RgnHandle region)
{
	PL_NotYetImplemented();
}
