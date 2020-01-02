#pragma once

#include <stdint.h>
#include "GpPixelFormat.h"
#include "PLErrorCodes.h"
#include "PLHandle.h"
#include "QDState.h"

struct PixMap;
struct Rect;

namespace PortabilityLayer
{
	class PixMapImpl;

	enum QDPortType
	{
		QDPortType_Invalid,

		QDPortType_DrawSurface,
		QDPortType_Window,
	};

	enum QDPortDirtyFlag
	{
		QDPortDirtyFlag_Size = 1,
		QDPortDirtyFlag_Contents = 2,
	};

	class QDPort
	{
	public:
		explicit QDPort(QDPortType portType);
		~QDPort();

		PLError_t Init(const Rect &rect, GpPixelFormat_t pixelFormat);
		QDPortType GetPortType() const;

		THandle<PixMap> GetPixMap() const;
		const QDState *GetState() const;
		QDState *GetState();
		GpPixelFormat_t GetPixelFormat() const;
		Rect GetRect() const;

		bool Resize(const Rect &rect);

		bool IsDirty(uint32_t flag) const;
		void SetDirty(uint32_t flag);
		void ClearDirty(uint32_t flag);

#if GP_DEBUG_CONFIG
		void CheckPortSentinel() const;
#endif

	private:
		void DisposePixMap();

#if GP_DEBUG_CONFIG
		int32_t m_portSentinel;
#endif

		QDPortType m_portType;

		QDState m_state;
		THandle<PixMapImpl> m_pixMap;

		int16_t m_left;
		int16_t m_top;
		uint16_t m_width;
		uint16_t m_height;
		uint32_t m_dirtyFlags;
		GpPixelFormat_t m_pixelFormat;

		uint32_t m_debugID;
	};

	inline QDPortType QDPort::GetPortType() const
	{
		return m_portType;
	}
}
