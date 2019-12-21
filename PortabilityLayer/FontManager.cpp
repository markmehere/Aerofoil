#include "FontManager.h"

#include "HostFileSystem.h"
#include "HostFont.h"
#include "HostFontHandler.h"
#include "IOStream.h"

void PL_NotYetImplemented();

namespace PortabilityLayer
{
	class FontManagerImpl final : public FontManager
	{
	public:
		void Init() override;
		void Shutdown() override;

		static FontManagerImpl *GetInstance();

	private:
		FontManagerImpl();

		PortabilityLayer::HostFont *m_systemFont;

		static FontManagerImpl ms_instance;
	};

	void FontManagerImpl::Init()
	{
		m_systemFont = nullptr;

		if (IOStream *sysFontStream = HostFileSystem::GetInstance()->OpenFile(EVirtualDirectory_Fonts, "virtue.ttf", false, false))
		{
			HostFont *font = HostFontHandler::GetInstance()->LoadFont(sysFontStream);
			sysFontStream->Close();

			m_systemFont = font;
		}
	}

	void FontManagerImpl::Shutdown()
	{
		HostFontHandler *hfh = HostFontHandler::GetInstance();

		if (m_systemFont)
		{
			m_systemFont->Destroy();
			m_systemFont = nullptr;
		}
	}

	FontManagerImpl *FontManagerImpl::GetInstance()
	{
		return &ms_instance;
	}

	FontManagerImpl::FontManagerImpl()
		: m_systemFont(nullptr)
	{
	}

	FontManagerImpl FontManagerImpl::ms_instance;

	FontManager *FontManager::GetInstance()
	{
		return FontManagerImpl::GetInstance();
	}
}
