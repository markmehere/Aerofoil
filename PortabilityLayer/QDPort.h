#pragma once

#include <stdint.h>
#include "PixelFormat.h"
#include "QDState.h"

struct PixMap;
struct Rect;

namespace PortabilityLayer
{
	enum QDPortType
	{
		QDPortType_Invalid,

		QDPortType_CGraf,
		QDPortType_Window,
	};

	class QDPort
	{
	public:
		explicit QDPort(QDPortType portType);
		~QDPort();

		int Init(const Rect &rect, PixelFormat pixelFormat);
		QDPortType GetPortType() const;

		PixMap **GetPixMap() const;
		const QDState *GetState() const;
		QDState *GetState();
		PixelFormat GetPixelFormat() const;
		Rect GetRect() const;

	private:
		QDPortType m_portType;

		QDState m_state;
		PixMap **m_pixMap;

		int16_t m_left;
		int16_t m_top;
		uint16_t m_width;
		uint16_t m_height;
		PixelFormat m_pixelFormat;
	};

	inline QDPortType QDPort::GetPortType() const
	{
		return m_portType;
	}
}
