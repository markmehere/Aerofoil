#pragma once

namespace PortabilityLayer
{
	enum EVirtualDirectory
	{
		EVirtualDirectory_Unspecified = 0,

		EVirtualDirectory_ApplicationData = 1,
		EVirtualDirectory_GameData,
		EVirtualDirectory_UserData,
		EVirtualDirectory_Prefs,
		EVirtualDirectory_Fonts,
	};
}
