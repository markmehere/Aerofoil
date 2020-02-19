#pragma once

#if __cplusplus >= 199711L
#define GP_IS_CPP11 1
#else
#define GP_IS_CPP11 0
#endif

#if GP_IS_CPP11
#define GP_DELETED = delete
#define GP_STATIC_ASSERT(n) static_assert((n), "Static assert failed: " #n)
#else
#ifndef nullptr
#define nullptr 0
#endif

#ifndef override
#define override
#endif

#ifndef final
#define final
#endif

#define GP_DELETED

template<bool TCondition>
struct __GpStaticAssertHelper
{
};

template<>
struct __GpStaticAssertHelper<true>
{
	int staticAssertFailed;
};

#define GP_STATIC_ASSERT(n) ((void)(&static_cast<const __GpStaticAssertHelper<(n)>*>(nullptr)->staticAssertFailed))

#endif

static const size_t GP_SYSTEM_MEMORY_ALIGNMENT = 16;

#if !defined(NDEBUG)
#define GP_DEBUG_CONFIG	1
#endif
