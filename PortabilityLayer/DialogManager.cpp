#include "DialogManager.h"
#include "ResourceManager.h"
#include "PLArrayView.h"
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
	namespace SerializedDialogItemTypeCodes
	{
		enum SerializedDialogItemTypeCode
		{
			kUserItem = 0x00,
			kButton = 0x04,
			kCheckBox = 0x05,
			kRadioButton = 0x06,
			kCustomControl = 0x07,
			kLabel = 0x08,
			kEditBox = 0x10,
			kIcon = 0x20,
			kImage = 0x40,
		};
	}

	typedef SerializedDialogItemTypeCodes::SerializedDialogItemTypeCode SerializedDialogItemTypeCode_t;

	struct DialogTemplateItem
	{
		Rect m_rect;
		int16_t m_id;
		uint8_t m_serializedType;
		bool m_enabled;
		Str255 m_name;
	};

	class DialogItemImpl : public DialogItem
	{
	public:
		DialogItemImpl(const DialogTemplateItem &templateItem);
		virtual ~DialogItemImpl();

		Rect GetRect() const override;

		virtual bool Init() = 0;
		virtual void Destroy() = 0;

	protected:
		Rect m_rect;
		int16_t m_id;
		bool m_enabled;
		PascalStr<255> m_name;
	};

	template<class T>
	class DialogItemSpec : public DialogItemImpl
	{
	public:
		DialogItemSpec(const DialogTemplateItem &tmpl)
			: DialogItemImpl(tmpl)
		{
		}

		void Destroy() override
		{
			static_cast<T*>(this)->~T();
			free(static_cast<T*>(this));
		}

		static DialogItemSpec *Create(const DialogTemplateItem &tmpl)
		{
			void *storage = malloc(sizeof(T));
			if (!storage)
				return nullptr;

			T *item = new (storage) T(tmpl);

			DialogItemImpl *dItem = static_cast<DialogItemImpl*>(item);
			if (!dItem->Init())
			{
				dItem->Destroy();
				return nullptr;
			}

			return item;
		}
	};

	class DialogItem_EditBox final : public DialogItemSpec<DialogItem_EditBox>
	{
	public:
		explicit DialogItem_EditBox(const DialogTemplateItem &tmpl);
		bool Init() override;
	};

	class DialogItem_Label final : public DialogItemSpec<DialogItem_Label>
	{
	public:
		explicit DialogItem_Label(const DialogTemplateItem &tmpl);
		bool Init() override;
	};

	class DialogItem_Unknown final : public DialogItemSpec<DialogItem_Unknown>
	{
	public:
		explicit DialogItem_Unknown(const DialogTemplateItem &tmpl);
		bool Init() override;
	};

	class DialogTemplate final
	{
	public:
		DialogTemplate(DialogTemplateItem *itemStorage, size_t numItems);
		void DeserializeItems(const uint8_t *data);
		void Destroy();

		ArrayView<const DialogTemplateItem> GetItems() const;

	private:
		DialogTemplateItem *m_items;
		size_t m_numItems;
	};

	class DialogImpl final : public Dialog
	{
	public:
		void Destroy() override;
		Window *GetWindow() const override;
		ArrayView<DialogItem*const> GetItems() const override;

		bool Populate(DialogTemplate *tmpl);

		static DialogImpl *Create(Window *window, size_t numItems);

	private:
		explicit DialogImpl(Window *window, DialogItem **items, size_t numItems);
		~DialogImpl();

		Window *m_window;
		DialogItem **m_items;
		size_t m_numItems;
	};


	DialogItemImpl::DialogItemImpl(const DialogTemplateItem &templateItem)
		: m_enabled(templateItem.m_enabled)
		, m_id(templateItem.m_id)
		, m_name(PLPasStr(templateItem.m_name))
		, m_rect(templateItem.m_rect)
	{
	}

	DialogItemImpl::~DialogItemImpl()
	{
	}

	Rect DialogItemImpl::GetRect() const
	{
		return m_rect;
	}

	DialogItem_EditBox::DialogItem_EditBox(const DialogTemplateItem &tmpl)
		: DialogItemSpec<DialogItem_EditBox>(tmpl)
	{
	}

	bool DialogItem_EditBox::Init()
	{
		return true;
	}

	DialogItem_Label::DialogItem_Label(const DialogTemplateItem &tmpl)
		: DialogItemSpec<DialogItem_Label>(tmpl)
	{
	}

	bool DialogItem_Label::Init()
	{
		return true;
	}

	DialogItem_Unknown::DialogItem_Unknown(const DialogTemplateItem &tmpl)
		: DialogItemSpec<DialogItem_Unknown>(tmpl)
	{
	}

	bool DialogItem_Unknown::Init()
	{
		return true;
	}

	DialogTemplate::DialogTemplate(DialogTemplateItem *itemStorage, size_t numItems)
		: m_items(itemStorage)
		, m_numItems(numItems)
	{
	}

	void DialogTemplate::DeserializeItems(const uint8_t *data)
	{
		for (size_t i = 0; i < m_numItems; i++)
		{
			BERect itemRect;
			uint8_t itemType;

			data += 4;	// Unused

			memcpy(&itemRect, data, 8);
			data += 8;

			itemType = *data;
			data++;

			uint8_t nameLength = *data;
			data++;

			const uint8_t *nameBytes = data;

			size_t nameLengthPadded = nameLength;
			if ((nameLength & 1) == 1)
				nameLengthPadded++;

			data += nameLengthPadded;

			DialogTemplateItem &item = m_items[i];
			item.m_rect = itemRect.ToRect();
			item.m_id = 0;
			item.m_serializedType = (itemType & 0x7f);
			item.m_enabled = ((itemType & 0x80) == 0);
			item.m_name[0] = nameLength;
			memcpy(item.m_name + 1, nameBytes, nameLength);

			if (item.m_serializedType == SerializedDialogItemTypeCodes::kCustomControl || item.m_serializedType == SerializedDialogItemTypeCodes::kImage || item.m_serializedType == SerializedDialogItemTypeCodes::kIcon)
			{
				memcpy(&item.m_id, item.m_name + 1, 2);
				ByteSwap::BigInt16(item.m_id);
			}
		}
	}

	void DialogTemplate::Destroy()
	{
		this->~DialogTemplate();
		free(this);
	}

	ArrayView<const DialogTemplateItem> DialogTemplate::GetItems() const
	{
		return ArrayView<const DialogTemplateItem>(m_items, m_numItems);
	}

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

	ArrayView<DialogItem*const> DialogImpl::GetItems() const
	{
		ArrayView<DialogItem*const> iter(m_items, m_numItems);
		return ArrayView<DialogItem*const>(m_items, m_numItems);
	}

	bool DialogImpl::Populate(DialogTemplate *tmpl)
	{
		ArrayView<const DialogTemplateItem> templateItems = tmpl->GetItems();

		const size_t numItems = templateItems.Count();

		for (size_t i = 0; i < numItems; i++)
		{
			const DialogTemplateItem &templateItem = templateItems[i];

			DialogItem *ditem = nullptr;

			switch (templateItem.m_serializedType)
			{
			case SerializedDialogItemTypeCodes::kLabel:
				ditem = DialogItem_Label::Create(templateItem);
				break;
			case SerializedDialogItemTypeCodes::kEditBox:
				ditem = DialogItem_EditBox::Create(templateItem);
				break;
			default:
				ditem = DialogItem_Unknown::Create(templateItem);
				break;
			}

			if (!ditem)
				return false;

			m_items[i] = ditem;
		}

		return true;
	}

	DialogImpl *DialogImpl::Create(Window *window, size_t numItems)
	{
		size_t alignedSize = sizeof(DialogImpl) + PL_SYSTEM_MEMORY_ALIGNMENT + 1;
		alignedSize -= alignedSize % PL_SYSTEM_MEMORY_ALIGNMENT;

		const size_t itemsSize = sizeof(DialogItemImpl) * numItems;

		void *storage = malloc(alignedSize + itemsSize);
		if (!storage)
			return nullptr;

		DialogItem **itemsList = reinterpret_cast<DialogItem **>(static_cast<uint8_t*>(storage) + alignedSize);
		for (size_t i = 0; i < numItems; i++)
			itemsList[i] = nullptr;

		return new (storage) DialogImpl(window, itemsList, numItems);
	}

	DialogImpl::DialogImpl(Window *window, DialogItem **itemsList, size_t numItems)
		: m_window(window)
		, m_items(itemsList)
		, m_numItems(numItems)
	{
	}

	DialogImpl::~DialogImpl()
	{
		for (size_t i = 0; i < m_numItems; i++)
		{
			if (DialogItem *item = m_items[i])
				static_cast<DialogItemImpl*>(item)->Destroy();
		}
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
		DialogTemplate *LoadDialogTemplate(int16_t resID);

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

		DialogTemplate *dtemplate = LoadDialogTemplate(header.m_itemsResID);
		const size_t numItems = (dtemplate == nullptr) ? 0 : dtemplate->GetItems().Count();

		if (!rect.IsValid())
		{
			dtemplate->Destroy();
			return nullptr;
		}

		WindowManager *wm = PortabilityLayer::WindowManager::GetInstance();

		WindowDef wdef = WindowDef::Create(rect, 0, header.m_visible != 0, header.m_hasCloseBox != 0, header.m_referenceConstant, positionSpec, PLPasStr(titlePStr));
		Window *window = wm->CreateWindow(wdef);
		if (!window)
		{
			dtemplate->Destroy();
			return nullptr;
		}

		wm->PutWindowBehind(window, behindWindow);

		DialogImpl *dialog = DialogImpl::Create(window, numItems);

		if (!dialog)
		{
			dtemplate->Destroy();
			wm->DestroyWindow(window);
			return nullptr;
		}

		if (!dialog->Populate(dtemplate))
		{
			dialog->Destroy();
			dtemplate->Destroy();
			wm->DestroyWindow(window);

			return nullptr;
		}

		return dialog;
	}

	DialogTemplate *DialogManagerImpl::LoadDialogTemplate(int16_t resID)
	{
		ResourceManager *rm = ResourceManager::GetInstance();

		THandle<uint8_t> dtemplateH = rm->GetResource('DITL', resID).StaticCast<uint8_t>();

		if (!dtemplateH)
			return nullptr;

		uint16_t numItems;
		memcpy(&numItems, *dtemplateH, 2);
		ByteSwap::BigUInt16(numItems);

		size_t dtlAlignedSize = sizeof(DialogTemplate) + PL_SYSTEM_MEMORY_ALIGNMENT - 1;
		dtlAlignedSize -= dtlAlignedSize % PL_SYSTEM_MEMORY_ALIGNMENT;

		const size_t dtlItemSize = sizeof(DialogTemplateItem) * numItems;

		void *storage = malloc(dtlAlignedSize + dtlItemSize);
		if (!storage)
		{
			dtemplateH.Dispose();
			return nullptr;
		}

		uint8_t *itemsLoc = static_cast<uint8_t*>(storage) + dtlAlignedSize;

		DialogTemplate *dtemplate = new (storage) DialogTemplate(reinterpret_cast<DialogTemplateItem*>(itemsLoc), numItems);
		dtemplate->DeserializeItems((*dtemplateH) + 2);

		dtemplateH.Dispose();

		return dtemplate;
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
