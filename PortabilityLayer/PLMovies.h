#pragma once
#ifndef __PL_MOVIES_H__
#define __PL_MOVIES_H__

#include "PLApplication.h"

struct UserDataObject;
struct TimeBaseObject;
struct MovieObject;

struct UserDataBackingStorage
{
};

typedef int64_t TimeValue;

struct TimeRecord
{
};

enum MovieFlags
{
	newMovieActive = 1,
	flushFromRam = 2,
};

enum TimeBaseFlags
{
	loopTimeBase = 1,
};

typedef UserDataObject *UserData;
typedef TimeBaseObject *TimeBase;
typedef MovieObject *Movie;


OSErr EnterMovies();

UserData GetMovieUserData(Movie movie);
int CountUserDataType(UserData userData, UInt32 type);
OSErr RemoveUserData(UserData userData, UInt32 type, int index);	// Index is 1-based
OSErr AddUserData(UserData userData, Handle data, UInt32 type);
OSErr OpenMovieFile(const FSSpec *fsSpec, short *outRefNum, int permissions);
OSErr NewMovieFromFile(Movie *movie, short refNum, const short *optResId, StringPtr resName, int flags, Boolean *unused);
OSErr CloseMovieFile(short refNum);
OSErr GoToBeginningOfMovie(Movie movie);
OSErr LoadMovieIntoRam(Movie movie, TimeValue time, TimeValue duration, int flags);
TimeValue GetMovieTime(Movie movie, TimeRecord *outCurrentTime);
TimeValue GetMovieDuration(Movie movie);
OSErr PrerollMovie(Movie movie, TimeValue time, UInt32 rate);
TimeBase GetMovieTimeBase(Movie movie);
OSErr SetTimeBaseFlags(TimeBase timeBase, int flags);
void SetMovieMasterTimeBase(Movie movie, TimeBase timeBase, void *unused);
void GetMovieBox(Movie movie, Rect *rect);
void StopMovie(Movie movie);
void DisposeMovie(Movie movie);
void SetMovieGWorld(Movie movie, CGrafPtr graf, void *unknown);
void SetMovieActive(Movie movie, Boolean active);
void StartMovie(Movie movie);
void MoviesTask(Movie movie, int unknown);
void SetMovieBox(Movie movie, const Rect *rect);
void SetMovieDisplayClipRgn(Movie movie, const Rect *rect);

#endif
