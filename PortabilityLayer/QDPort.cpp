#include "QDPort.h"
#include "PLErrorCodes.h"
#include "MemoryManager.h"
#include "MMHandleBlock.h"
#include "QDPixMap.h"

namespace PortabilityLayer
{
	static uint32_t gs_nextQDPortDebugID = 0;

	QDPort::QDPort(QDPortType portType)
		: m_portType(portType)
		, m_left(0)
		, m_top(0)
		, m_width(0)
		, m_height(0)
		, m_pixMap(nullptr)
		, m_pixelFormat(GpPixelFormats::kInvalid)
		, m_dirtyFlags(0)
		, m_debugID(gs_nextQDPortDebugID++)
	{
	}

	QDPort::~QDPort()
	{
		DisposePixMap();
	}

	void QDPort::DisposePixMap()
	{
		if (m_pixMap)
		{
			if (*m_pixMap)
				static_cast<PixMapImpl*>(*m_pixMap)->~PixMapImpl();
			MemoryManager::GetInstance()->ReleaseHandle(reinterpret_cast<MMHandleBlock*>(m_pixMap));
		}
	}

	PLError_t QDPort::Init(const Rect &rect, GpPixelFormat_t pixelFormat)
	{
		m_pixMap = nullptr;
		m_pixelFormat = pixelFormat;

		if (!Resize(rect))
			return PLErrors::kOutOfMemory;

		return PLErrors::kNone;
	}

	bool QDPort::Resize(const Rect &rect)
	{
		const uint16_t width = static_cast<uint16_t>(rect.right - rect.left);
		const uint16_t height = static_cast<uint16_t>(rect.bottom - rect.top);

		size_t pixMapSize = PixMapImpl::SizeForDimensions(width, height, m_pixelFormat);
		if (pixMapSize == 0)
			return false;

		MMHandleBlock *pmBlock = MemoryManager::GetInstance()->AllocHandle(pixMapSize);
		if (!pmBlock)
			return false;

		memset(pmBlock->m_contents, 0, pixMapSize);

		SetDirty(QDPortDirtyFlag_Size | QDPortDirtyFlag_Contents);

		m_left = rect.left;
		m_top = rect.top;
		m_width = width;
		m_height = height;

		new (pmBlock->m_contents) PixMapImpl(m_left, m_top, width, height, m_pixelFormat);

		DisposePixMap();

		m_pixMap = reinterpret_cast<PixMap**>(&pmBlock->m_contents);

		return true;
	}

	bool QDPort::IsDirty(uint32_t flag) const
	{
		return (m_dirtyFlags & flag) != 0;
	}

	void QDPort::SetDirty(uint32_t flag)
	{
		m_dirtyFlags |= flag;
	}

	void QDPort::ClearDirty(uint32_t flag)
	{
		m_dirtyFlags &= ~flag;
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

	GpPixelFormat_t QDPort::GetPixelFormat() const
	{
		return m_pixelFormat;
	}

	Rect QDPort::GetRect() const
	{
		return Rect::Create(m_top, m_left, m_top + m_height, m_left + m_width);
	}
}
