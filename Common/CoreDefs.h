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
#endif


static const size_t PL_SYSTEM_MEMORY_ALIGNMENT = 16;


#endif
