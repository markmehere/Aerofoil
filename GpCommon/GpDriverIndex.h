#pragma once

#include "CoreDefs.h"

#include <stdint.h>

namespace GpDriverIDs
{
	enum GpDriverID
	{
		kAudio,
		kFileSystem,
		kDisplay,
		kLog,
		kInput,
		kSystemServices,
		kFont,
		kEventQueue,
		kAlloc,

		kCount
	};
}

typedef GpDriverIDs::GpDriverID GpDriverID_t;

template<int T>
struct GpDriverIndex
{
};

#define GP_DEFINE_DRIVER(driverID, type)	\
	struct type;\
	template<>\
	struct GpDriverIndex<GpDriverIDs::driverID>\
	{\
		typedef type Type_t;\
		static const bool kIsMultiDriver = false;\
	}

#define GP_DEFINE_MULTI_DRIVER(driverID, type)	\
	struct type;\
	template<>\
	struct GpDriverIndex<GpDriverIDs::driverID>\
	{\
		typedef type Type_t;\
		static const bool kIsMultiDriver = true;\
	}

GP_DEFINE_DRIVER(kAudio, IGpAudioDriver);
GP_DEFINE_DRIVER(kFileSystem, IGpFileSystem);
GP_DEFINE_DRIVER(kDisplay, IGpDisplayDriver);
GP_DEFINE_DRIVER(kLog, IGpLogDriver);
GP_DEFINE_MULTI_DRIVER(kInput, IGpInputDriver);
GP_DEFINE_DRIVER(kSystemServices, IGpSystemServices);
GP_DEFINE_DRIVER(kFont, IGpFontHandler);
GP_DEFINE_DRIVER(kEventQueue, IGpVOSEventQueue);
GP_DEFINE_DRIVER(kAlloc, IGpAllocator);

struct GpDriverCollection
{
	GpDriverCollection();

	template<GpDriverID_t T>
	void SetDriver(typename GpDriverIndex<T>::Type_t *driver);

	template<GpDriverID_t T>
	void SetDrivers(typename GpDriverIndex<T>::Type_t *const* drivers, size_t numDrivers);

	template<GpDriverID_t T>
	typename GpDriverIndex<T>::Type_t *GetDriver() const;

	template<GpDriverID_t T>
	typename GpDriverIndex<T>::Type_t *GetDriver(size_t index) const;

	template<GpDriverID_t T>
	size_t GetDriverCount() const;

private:
	struct DriverEntry
	{
		void *m_value;
		size_t m_numDrivers;
	};

	DriverEntry m_drivers[GpDriverIDs::kCount];
};

inline GpDriverCollection::GpDriverCollection()
{
	for (int i = 0; i < GpDriverIDs::kCount; i++)
	{
		this->m_drivers[i].m_value = nullptr;
		this->m_drivers[i].m_numDrivers = 0;
	}
}

template<GpDriverID_t T>
void GpDriverCollection::SetDriver(typename GpDriverIndex<T>::Type_t *driver)
{
	GP_STATIC_ASSERT(!GpDriverIndex<T>::kIsMultiDriver);
	m_drivers[T].m_numDrivers = 1;
	m_drivers[T].m_value = driver;
}

template<GpDriverID_t T>
void GpDriverCollection::SetDrivers(typename GpDriverIndex<T>::Type_t *const* drivers, size_t numDrivers)
{
	GP_STATIC_ASSERT(GpDriverIndex<T>::kIsMultiDriver);
	m_drivers[T].m_numDrivers = numDrivers;
	m_drivers[T].m_value = const_cast<typename GpDriverIndex<T>::Type_t **>(drivers);
}

template<GpDriverID_t T>
inline typename GpDriverIndex<T>::Type_t *GpDriverCollection::GetDriver() const
{
	GP_STATIC_ASSERT(!GpDriverIndex<T>::kIsMultiDriver);
	return static_cast<typename GpDriverIndex<T>::Type_t*>(this->m_drivers[T].m_value);
}

template<GpDriverID_t T>
inline typename GpDriverIndex<T>::Type_t *GpDriverCollection::GetDriver(size_t index) const
{
	GP_STATIC_ASSERT(GpDriverIndex<T>::kIsMultiDriver);
	return static_cast<typename GpDriverIndex<T>::Type_t*const*>(this->m_drivers[T].m_value)[index];
}


template<GpDriverID_t T>
size_t GpDriverCollection::GetDriverCount() const
{
	GP_STATIC_ASSERT(GpDriverIndex<T>::kIsMultiDriver);
	return this->m_drivers[T].m_numDrivers;
}
