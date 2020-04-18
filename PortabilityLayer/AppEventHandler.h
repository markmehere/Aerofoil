#pragma once

namespace PortabilityLayer
{
	struct IAppEventHandler
	{
		virtual void OnQuit() = 0;
	};

	class AppEventHandler
	{
	public:
		static IAppEventHandler *GetInstance();
		static void SetInstance(IAppEventHandler *instance);

	private:
		static IAppEventHandler *ms_instance;
	};
}
