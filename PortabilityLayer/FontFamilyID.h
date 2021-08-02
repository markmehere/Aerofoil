#pragma once

namespace PortabilityLayer
{
	namespace FontFamilyIDs
	{
		enum FontFamilyID
		{
			kSystemSymbols,
			kSystem,
			kApplication,
			kMonospace,
			kHandwriting,

			kCount
		};
	}

	typedef FontFamilyIDs::FontFamilyID FontFamilyID_t;
}
