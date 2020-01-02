#include "PLHandle.h"

#include "MemoryManager.h"

void THandleBase::Dispose() const
{
	if (m_hdl)
		PortabilityLayer::MemoryManager::GetInstance()->ReleaseHandle(m_hdl);
}
