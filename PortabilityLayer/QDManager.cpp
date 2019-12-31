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
		QDPort *GetPort() const override;
		void SetPort(QDPort *gw) override;
		PLError_t NewGWorld(DrawSurface **gw, GpPixelFormat_t pixelFormat, const Rect &bounds, ColorTable **colorTable) override;
		void DisposeGWorld(DrawSurface *gw) override;
		QDState *GetState() override;

		static QDManagerImpl *GetInstance();

	private:
		QDPort *m_port;

		static QDManagerImpl ms_instance;
	};

	QDManagerImpl::QDManagerImpl()
		: m_port(nullptr)
	{
	}

	void QDManagerImpl::Init()
	{
	}

	QDPort *QDManagerImpl::GetPort() const
	{
		return m_port;
	}

	void QDManagerImpl::SetPort(QDPort *gw)
	{
		m_port = gw;
	}

	PLError_t QDManagerImpl::NewGWorld(DrawSurface **gw, GpPixelFormat_t pixelFormat, const Rect &bounds, ColorTable **colorTable)
	{
		void *grafStorage = MemoryManager::GetInstance()->Alloc(sizeof(DrawSurface));
		if (!grafStorage)
			return PLErrors::kOutOfMemory;

		if (!bounds.IsValid())
			return PLErrors::kInvalidParameter;

		DrawSurface *graf = new (grafStorage) DrawSurface();
		PLError_t initError = graf->Init(bounds, pixelFormat);
		if (initError)
		{
			DisposeGWorld(graf);
			return initError;
		}

		*gw = graf;
		return PLErrors::kNone;
	}

	void QDManagerImpl::DisposeGWorld(DrawSurface *gw)
	{
		gw->~DrawSurface();
		MemoryManager::GetInstance()->Release(gw);
	}

	QDState *QDManagerImpl::GetState()
	{
		return m_port->GetState();
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
