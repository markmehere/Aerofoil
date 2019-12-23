#pragma once

namespace GpPixelFormats
{
	enum GpPixelFormat
	{
		kInvalid,

		k8BitStandard,
		k8BitCustom,
		kRGB555,
		kRGB24,
		kRGB32,
	};
}

typedef GpPixelFormats::GpPixelFormat GpPixelFormat_t;
