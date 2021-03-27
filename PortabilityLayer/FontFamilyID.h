#pragma once

namespace PortabilityLayer
{
	namespace FontFamilyIDs
	{
		enum FontFamilyID
		{
			kSystem,
			kApplication,
			kMonospace,
			kHandwriting,

			kCount
		};
	}

	typedef FontFamilyIDs::FontFamilyID FontFamilyID_t;
}
