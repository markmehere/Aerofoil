#pragma once

#include <stdint.h>

struct Dialog;
struct DialogTextSubstitutions;
struct Window;
struct TimeTaggedVOSEvent;

namespace PortabilityLayer
{
	class DialogImpl;
	class Widget;

	class DialogManager
	{
	public:
		virtual Dialog *LoadDialog(int16_t resID, Window *behindWindow, const DialogTextSubstitutions *substitutions) = 0;
		virtual int16_t DisplayAlert(int16_t alertResID, const DialogTextSubstitutions *substitutions) = 0;

		static DialogManager *GetInstance();
	};

	class DialogItem final
	{
	public:
		explicit DialogItem(Widget *widget);
		~DialogItem();

		Widget *GetWidget() const;

	private:
		friend class DialogImpl;

		Widget *m_widget;
	};
}
