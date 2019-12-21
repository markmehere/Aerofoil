#include "InputManager.h"

#include <string.h>

namespace PortabilityLayer
{
	class InputManagerImpl final : public InputManager
	{
	public:
		InputManagerImpl();

		void GetKeys(unsigned char *keys16) const override;

		static InputManagerImpl *GetInstance();

	private:
		unsigned char m_keys[16];

		static InputManagerImpl ms_instance;
	};

	void InputManagerImpl::GetKeys(unsigned char *keys16) const
	{
		memcpy(keys16, m_keys, 16);
	}

	InputManagerImpl *InputManagerImpl::GetInstance()
	{
		return &ms_instance;
	}

	InputManagerImpl::InputManagerImpl()
	{
		for (int i = 0; i < sizeof(m_keys) / sizeof(m_keys[0]); i++)
			m_keys[i] = 0;
	}

	InputManagerImpl InputManagerImpl::ms_instance;

	InputManager *InputManager::GetInstance()
	{
		return InputManagerImpl::GetInstance();
	}
}
