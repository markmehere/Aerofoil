#pragma once

#ifndef __PL_COREDEFS_H__
#define __PL_COREDEFS_H__

#if __cplusplus >= 199711L
#define PL_IS_CPP11 1
#else
#define PL_IS_CPP11 0
#endif

#if PL_IS_CPP11
#define PL_DELETED = delete
#define PL_STATIC_ASSERT(n) static_assert((n), "Static assert failed: " #n)
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

#define PL_DELETED

template<bool TCondition>
struct __PL_StaticAssertHelper
{
};

template<>
struct __PL_StaticAssertHelper<true>
{
	int staticAssertFailed;
};

#define PL_STATIC_ASSERT(n) ((void)(&static_cast<const __PL_StaticAssertHelper<(n)>*>(nullptr)->staticAssertFailed))

#endif

static const size_t PL_SYSTEM_MEMORY_ALIGNMENT = 16;

#endif
