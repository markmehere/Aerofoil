#include "QDManager.h"

#include "MemoryManager.h"
#include "PLCore.h"
#include "PLQDOffscreen.h"
#include "QDGraf.h"
#include "QDState.h"

#include <assert.h>

namespace PortabilityLayer
{
	class QDManagerImpl final : public QDManager
	{
	public:
		QDManagerImpl();

		void Init() override;
		void GetPort(QDPort **port, GDevice ***gdHandle) override;
		void SetPort(QDPort *gw, GDevice **gdHandle) override;
		int NewGWorld(CGraf **gw, int depth, const Rect &bounds, ColorTable **colorTable, GDevice **device, int flags) override;
		void DisposeGWorld(CGraf *gw) override;
		QDState *GetState() override;

		int DepthForPixelFormat(GpPixelFormat_t pixelFormat) const override;

		static QDManagerImpl *GetInstance();

	private:
		QDPort *m_port;
		GDHandle m_gdHandle;

		static QDManagerImpl ms_instance;
	};

	QDManagerImpl::QDManagerImpl()
		: m_port(nullptr)
		, m_gdHandle(nullptr)
	{
	}

	void QDManagerImpl::Init()
	{
	}

	void QDManagerImpl::GetPort(QDPort **port, GDevice ***gdHandle)
	{
		if (port)
			*port = m_port;
		if (gdHandle)
			*gdHandle = m_gdHandle;
	}

	void QDManagerImpl::SetPort(QDPort *gw, GDevice **gdHandle)
	{
		m_port = gw;
		m_gdHandle = gdHandle;
	}

	int QDManagerImpl::NewGWorld(CGraf **gw, int depth, const Rect &bounds, ColorTable **colorTable, GDevice **device, int flags)
	{
		GpPixelFormat_t pixelFormat;

		switch (depth)
		{
		case 1:
			pixelFormat = GpPixelFormats::kBW1;
			break;
		case 8:
			pixelFormat = (colorTable == nullptr) ? GpPixelFormats::k8BitStandard : GpPixelFormats::k8BitCustom;
			break;
		case 16:
			pixelFormat = GpPixelFormats::kRGB555;
			break;
		case 32:
			pixelFormat = GpPixelFormats::kRGB32;
			break;
		default:
			return genericErr;
		}

		void *grafStorage = MemoryManager::GetInstance()->Alloc(sizeof(CGraf));
		if (!grafStorage)
			return mFulErr;

		if (!bounds.IsValid())
			return genericErr;

		CGraf *graf = new (grafStorage) CGraf();
		int initError = graf->Init(bounds, pixelFormat);
		if (initError)
		{
			DisposeGWorld(graf);
			return initError;
		}

		*gw = graf;
		return noErr;
	}

	void QDManagerImpl::DisposeGWorld(CGraf *gw)
	{
		gw->~CGraf();
		MemoryManager::GetInstance()->Release(gw);
	}

	QDState *QDManagerImpl::GetState()
	{
		return m_port->GetState();
	}

	int QDManagerImpl::DepthForPixelFormat(GpPixelFormat_t pixelFormat) const
	{
		switch (pixelFormat)
		{
		case GpPixelFormats::k8BitStandard:
		case GpPixelFormats::k8BitCustom:
			return 8;
		case GpPixelFormats::kRGB555:
			return 16;
		case GpPixelFormats::kRGB24:
			return 24;
		case GpPixelFormats::kRGB32:
			return 32;
		default:
			assert(false);
			return 0;
		}
	}

	QDManagerImpl *QDManagerImpl::GetInstance()
	{
		return &ms_instance;
	}

	QDManagerImpl QDManagerImpl::ms_instance;

	QDManager *QDManager::GetInstance()
	{
		return QDManagerImpl::GetInstance();
	}
}
