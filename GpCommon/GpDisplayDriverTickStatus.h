#pragma once

namespace GpDisplayDriverTickStatuses
{
	enum GpDisplayDriverTickStatus
	{
		kOK = 0,
		kNonFatalFault = 1,
		kFatalFault = 2,

		kApplicationTerminated = 3,
	};
}

typedef GpDisplayDriverTickStatuses::GpDisplayDriverTickStatus GpDisplayDriverTickStatus_t;
