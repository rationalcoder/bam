#ifndef __BAM_H__
#define __BAM_H__

#include "bam_default_config.h"
#include "bam_config.h"

//{ Config asserts. Some things just don't have a default.
#ifndef BAM_ALLOCATE
    #error "no BAM_ALLOCATE defined"
#endif

#ifndef BAM_TEMP_ALLOCATE
    #error "no BAM_TEMP_ALLOCATE defined"
#endif
//}


//{ Standard library replacements
#if !defined(BAM_MEMCPY) || !defined(BAM_MEMSET) || !defined(BAM_MEMCMP)
    #include <string.h>
#endif

#ifndef BAM_MEMCPY
    #define BAM_MEMCPY ::memcpy
#endif

#ifndef BAM_MEMSET
    #define BAM_MEMSET ::memset
#endif

#ifndef BAM_MEMCMP
    #define BAM_MEMCMP ::memcmp
#endif

//}

#ifndef BAM_STB_SPRINTF_INCLUDE
    #define BAM_STB_SPRINTF_INCLUDE "stb_sprintf.h"
#endif

#ifndef BAM_REPLACE_INTS
    #include <stdint.h>
    #include "bam_ints.h"
#endif

#ifdef BAM_WANT_SHORT_INTS
    #include "bam_short_ints.h"
#endif

#include <new>
//void* operator new (bumm, void* ptr) { return ptr; }

#include BAM_STB_SPRINTF_INCLUDE
#include "bam_common.h"
#include "bam_memory.h"
#include "bam_buffer.h"
#endif

// Always allow people to plop an implementation down somewhere,
// regardless of include guards.
#ifdef BAM_IMPLEMENTATION
#include "bam_buffer.cpp"
#endif
