#include "PLMovies.h"

PLError_t EnterMovies()
{
	return PLErrors::kNone;
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

PLError_t RemoveUserData(UserData userData, UInt32 type, int index)
{
	PL_NotYetImplemented();
	return PLErrors::kNone;
}

PLError_t AddUserData(UserData userData, Handle data, UInt32 type)
{
	PL_NotYetImplemented();
	return PLErrors::kNone;
}

PLError_t OpenMovieFile(const VFileSpec &fsSpec, short *outRefNum, int permissions)
{
	PL_NotYetImplemented_TODO("Movies");
	return PLErrors::kNone;
}

PLError_t NewMovieFromFile(Movie *movie, short refNum, const short *optResId, StringPtr resName, int flags, Boolean *unused)
{
	PL_NotYetImplemented_TODO("Movies");
	return PLErrors::kNone;
}

PLError_t CloseMovieFile(short refNum)
{
	PL_NotYetImplemented();
	return PLErrors::kNone;
}

PLError_t GoToBeginningOfMovie(Movie movie)
{
	PL_NotYetImplemented();
	return PLErrors::kNone;
}

PLError_t LoadMovieIntoRam(Movie movie, TimeValue time, TimeValue duration, int flags)
{
	PL_NotYetImplemented();
	return PLErrors::kNone;
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

PLError_t PrerollMovie(Movie movie, TimeValue time, UInt32 rate)
{
	PL_NotYetImplemented();
	return PLErrors::kNone;
}

TimeBase GetMovieTimeBase(Movie movie)
{
	PL_NotYetImplemented();
	return nullptr;
}

PLError_t SetTimeBaseFlags(TimeBase timeBase, int flags)
{
	PL_NotYetImplemented();
	return PLErrors::kNone;
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

void SetMovieGWorld(Movie movie, DrawSurface *graf, void *unknown)
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

void SetMovieDisplayClipRgn(Movie movie, const Rect *rect)
{
	PL_NotYetImplemented();
}
