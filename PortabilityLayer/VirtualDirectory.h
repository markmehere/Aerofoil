#pragma once

namespace PortabilityLayer
{
	namespace VirtualDirectories
	{
		enum VirtualDirectory
		{
			kUnspecified = 0,

			kApplicationData = 1,
			kGameData,
			kUserData,
			kPrefs,
			kFonts,
			kCursors,
			kHighScores,
		};
	}

	typedef VirtualDirectories::VirtualDirectory VirtualDirectory_t;
}
