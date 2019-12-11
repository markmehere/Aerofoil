#include "QDPort.h"
#include "PLErrorCodes.h"
#include "MemoryManager.h"
#include "MMHandleBlock.h"
#include "QDPixMap.h"

namespace PortabilityLayer
{
	QDPort::QDPort(QDPortType portType)
		: m_portType(portType)
		, m_left(0)
		, m_top(0)
		, m_width(0)
		, m_height(0)
		, m_pixMap(nullptr)
		, m_pixelFormat(PixelFormat_Invalid)
	{
	}

	QDPort::~QDPort()
	{
		if (m_pixMap)
		{
			if (*m_pixMap)
				static_cast<PixMapImpl*>(*m_pixMap)->~PixMapImpl();
			MemoryManager::GetInstance()->ReleaseHandle(reinterpret_cast<MMHandleBlock*>(m_pixMap));
		}
	}

	int QDPort::Init(const Rect &rect, PixelFormat pixelFormat)
	{
		m_left = rect.left;
		m_top = rect.top;
		m_width = static_cast<uint16_t>(rect.right - rect.left);
		m_height = static_cast<uint16_t>(rect.bottom - rect.top);
		m_pixMap = nullptr;
		m_pixelFormat = pixelFormat;

		size_t pixMapSize = PixMapImpl::SizeForDimensions(m_width, m_height, m_pixelFormat);
		if (pixMapSize == 0)
			return mFulErr;

		MMHandleBlock *pmBlock = MemoryManager::GetInstance()->AllocHandle(pixMapSize);
		if (!pmBlock)
			return mFulErr;

		new (pmBlock->m_contents) PixMapImpl(m_left, m_top, m_width, m_height, m_pixelFormat);
		m_pixMap = reinterpret_cast<PixMap**>(&pmBlock->m_contents);

		return noErr;
	}

	PixMap **QDPort::GetPixMap() const
	{
		return m_pixMap;
	}

	const QDState *QDPort::GetState() const
	{
		return &m_state;
	}

	QDState *QDPort::GetState()
	{
		return &m_state;
	}

	PixelFormat QDPort::GetPixelFormat() const
	{
		return m_pixelFormat;
	}

	Rect QDPort::GetRect() const
	{
		return Rect::Create(0, 0, m_height, m_width);
	}
}
