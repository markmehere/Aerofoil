#pragma once
#ifndef __PL_ERROR_CODES_H__
#define __PL_ERROR_CODES_H__


namespace PLErrors
{
	enum PLError
	{
		kNone = 0,

		kInvalidParameter,

		kFileHandlesExhausted,
		kBadFileName,
		kFileNotFound,
		kAccessDenied,

		kOutOfMemory,

		kAudioError,

		kIOError,

		kUserCancelled_TEMP,
	};
}

typedef PLErrors::PLError PLError_t;

#endif
