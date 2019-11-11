#include "AEManager.h"
#include "AEHandlerDesc.h"

#include <vector>

namespace PortabilityLayer
{
	class AEManagerImpl final : public AEManager
	{
	public:
		AEManagerImpl();

		void Init() override;
		void Shutdown() override;

		void InstallEventHandler(AEEventClass eventClass, AEEventID eventID, AEEventHandler handler, uint32_t ref, bool isSysHandler) override;
		void SetInteractAllowed(AEInteractAllowed interactAllowed) override;

		static AEManagerImpl *GetInstance();

	private:
		std::vector<AEHandlerDesc> m_handlers;
		AEInteractAllowed m_interactAllowed;

		static AEManagerImpl ms_instance;
	};

	AEManagerImpl::AEManagerImpl()
		: m_interactAllowed(kAEInteractUnknown)
	{
	}

	void AEManagerImpl::Init()
	{
		m_interactAllowed = kAEInteractUnknown;
	}

	void AEManagerImpl::Shutdown()
	{
		m_handlers.clear();
	}

	void AEManagerImpl::InstallEventHandler(AEEventClass eventClass, AEEventID eventID, AEEventHandler handler, uint32_t ref, bool isSysHandler)
	{
		AEHandlerDesc desc;
		desc.m_eventClass = eventClass;
		desc.m_eventID = eventID;
		desc.m_handler = handler;
		desc.m_ref = ref;

		m_handlers.push_back(desc);
	}

	void AEManagerImpl::SetInteractAllowed(AEInteractAllowed interactAllowed)
	{
		m_interactAllowed = interactAllowed;
	}

	AEManagerImpl *AEManagerImpl::GetInstance()
	{
		return &ms_instance;
	}

	AEManagerImpl AEManagerImpl::ms_instance;


	AEManager *AEManager::GetInstance()
	{
		return AEManagerImpl::GetInstance();
	}
}


