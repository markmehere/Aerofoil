#pragma once

namespace PortabilityLayer
{
	class FontManager
	{
	public:
		virtual void Init() = 0;
		virtual void Shutdown() = 0;

		static FontManager *GetInstance();
	};
}
