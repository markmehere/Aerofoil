#include "QDManager.h"

#include "MemoryManager.h"
#include "PLCore.h"
#include "PLQDOffscreen.h"
#include "QDGraf.h"

#include <assert.h>

namespace PortabilityLayer
{
	class QDManagerImpl final : public QDManager
	{
	public:
		QDManagerImpl();

		void Init() override;
		PLError_t NewGWorld(DrawSurface **gw, GpPixelFormat_t pixelFormat, const Rect &bounds) override;
		void DisposeGWorld(DrawSurface *gw) override;

		static QDManagerImpl *GetInstance();

	private:
		static QDManagerImpl ms_instance;
	};

	QDManagerImpl::QDManagerImpl()
	{
	}

	void QDManagerImpl::Init()
	{
	}

	PLError_t QDManagerImpl::NewGWorld(DrawSurface **gw, GpPixelFormat_t pixelFormat, const Rect &bounds)
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
