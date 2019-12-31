#include "DialogManager.h"
#include "ResourceManager.h"
#include "PLDialogs.h"
#include "PLBigEndian.h"
#include "PLPasStr.h"
#include "ResTypeID.h"
#include "SharedTypes.h"
#include "WindowDef.h"
#include "WindowManager.h"

#include <stdlib.h>
#include <new>

namespace PortabilityLayer
{
	class DialogImpl final : public Dialog
	{
	public:
		void Destroy() override;
		Window *GetWindow() const override;

		static DialogImpl *Create(Window *window);

	private:
		explicit DialogImpl(Window *window);
		~DialogImpl();

		Window *m_window;
	};

	void DialogImpl::Destroy()
	{
		PortabilityLayer::WindowManager::GetInstance()->DestroyWindow(m_window);

		this->~DialogImpl();
		free(this);
	}

	Window *DialogImpl::GetWindow() const
	{
		return m_window;
	}

	DialogImpl *DialogImpl::Create(Window *window)
	{
		void *storage = malloc(sizeof(DialogImpl));
		if (!storage)
			return nullptr;

		return new (storage) DialogImpl(window);
	}

	DialogImpl::DialogImpl(Window *window)
		: m_window(window)
	{
	}

	DialogImpl::~DialogImpl()
	{
	}

	// DLOG resource format:
	// DialogResourceDataHeader
	// Variable-length PStr: Title
	// Optional: Positioning (2 byte mask)

	struct DialogResourceDataHeader
	{
		BERect m_rect;
		BEInt16_t m_style;
		uint8_t m_visible;
		uint8_t m_unusedA;
		uint8_t m_hasCloseBox;
		uint8_t m_unusedB;
		BEUInt32_t m_referenceConstant;
		BEInt16_t m_itemsResID;
	};

	class DialogManagerImpl final : public DialogManager
	{
	public:
		Dialog *LoadDialog(int16_t resID, Window *behindWindow) override;

		static DialogManagerImpl *GetInstance();

	private:
		static DialogManagerImpl ms_instance;
	};

	Dialog *DialogManagerImpl::LoadDialog(int16_t resID, Window *behindWindow)
	{
		ResourceManager *rm = ResourceManager::GetInstance();

		THandle<uint8_t> dlogH = rm->GetResource('DLOG', resID).StaticCast<uint8_t>();
		const uint8_t *dlogData = *dlogH;
		const uint8_t *dlogDataEnd = dlogData + dlogH.MMBlock()->m_size;

		DialogResourceDataHeader header;
		memcpy(&header, dlogData, sizeof(header));

		const uint8_t *titlePStr = dlogData + sizeof(header);
		const uint8_t *positioningData = titlePStr + 1 + titlePStr[0];	// May be OOB

		BEUInt16_t positionSpec(0);
		if (positioningData != dlogDataEnd)
			memcpy(&positionSpec, positioningData, 2);

		const Rect rect = header.m_rect.ToRect();
		const int16_t style = header.m_style;

		dlogH.Dispose();

		if (!rect.IsValid())
			return nullptr;

		WindowManager *wm = PortabilityLayer::WindowManager::GetInstance();

		WindowDef wdef = WindowDef::Create(rect, 0, header.m_visible != 0, header.m_hasCloseBox != 0, header.m_referenceConstant, positionSpec, PLPasStr(titlePStr));
		Window *window = wm->CreateWindow(wdef);
		if (!window)
			return nullptr;

		wm->PutWindowBehind(window, behindWindow);

		THandle<uint8_t> dtemplateH = rm->GetResource('DITL', header.m_itemsResID).StaticCast<uint8_t>();

		Dialog *dialog = DialogImpl::Create(window);

		if (!dialog)
		{
			wm->DestroyWindow(window);
			return nullptr;
		}

		return dialog;
	}

	DialogManagerImpl *DialogManagerImpl::GetInstance()
	{
		return &ms_instance;
	}

	DialogManagerImpl DialogManagerImpl::ms_instance;

	DialogManager *DialogManager::GetInstance()
	{
		return DialogManagerImpl::GetInstance();
	}
}
