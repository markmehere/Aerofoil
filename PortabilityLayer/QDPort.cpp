#include "QDPort.h"
#include "PLErrorCodes.h"
#include "PLHandle.h"
#include "MemoryManager.h"
#include "MMHandleBlock.h"
#include "QDManager.h"
#include "QDPixMap.h"

#if GP_DEBUG_CONFIG
#include <assert.h>

static const int32_t kQDPortSentinelValue = 0x222a1877;	// Completely arbitrary number
#endif

namespace PortabilityLayer
{
	static uint32_t gs_nextQDPortDebugID = 0;

	QDPort::QDPort(QDPortType portType)
		: m_portType(portType)
		, m_left(0)
		, m_top(0)
		, m_width(0)
		, m_height(0)
		, m_pixelFormat(GpPixelFormats::kInvalid)
		, m_dirtyFlags(0)
		, m_debugID(gs_nextQDPortDebugID++)
#if GP_DEBUG_CONFIG
		, m_portSentinel(kQDPortSentinelValue)
#endif
	{
	}

	QDPort::~QDPort()
	{
#if GP_DEBUG_CONFIG
		// Detach the port BEFORE destroying it!!
		assert(&PortabilityLayer::QDManager::GetInstance()->GetPort()->m_port != this);
#endif

		DisposePixMap();
	}

	void QDPort::DisposePixMap()
	{
		if (m_pixMap)
			PixMapImpl::Destroy(m_pixMap);
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
		if (!rect.IsValid())
			return false;

		THandle<PixMapImpl> newPixMap = PixMapImpl::Create(rect, m_pixelFormat);

		if (!newPixMap)
			return false;

		SetDirty(QDPortDirtyFlag_Size | QDPortDirtyFlag_Contents);

		m_left = rect.left;
		m_top = rect.top;
		m_width = rect.Width();
		m_height = rect.Height();

		DisposePixMap();

		m_pixMap = newPixMap;

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

	THandle<PixMap> QDPort::GetPixMap() const
	{
		return m_pixMap.ImplicitCast<PixMap>();
	}

	GpPixelFormat_t QDPort::GetPixelFormat() const
	{
		return m_pixelFormat;
	}

	Rect QDPort::GetRect() const
	{
		return Rect::Create(m_top, m_left, m_top + m_height, m_left + m_width);
	}

#if GP_DEBUG_CONFIG
	void QDPort::CheckPortSentinel() const
	{
		assert(m_portSentinel == kQDPortSentinelValue);
	}
#endif
}
