#pragma once

namespace GpDisplayDriverTickStatuses
{
	enum GpDisplayDriverTickStatus
	{
		kOK = 0,
		kNonFatalFault = 1,
		kFatalFault = 2,

		kApplicationTerminated = 3,

		kSynchronizing = 4,
	};
}

typedef GpDisplayDriverTickStatuses::GpDisplayDriverTickStatus GpDisplayDriverTickStatus_t;
