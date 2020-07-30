#ifndef __BAM_H__
#define __BAM_H__

#include "bam_config.h" // provided by client
#include "bam_default_config.h"

// Can't really replace errno...
#include <errno.h>

#include BAM_STB_SPRINTF_INCLUDE
#include "bam_types.h"
#include "bam_shared.h"
#include "bam_platform.h"
#include "bam_common.h"
#include "bam_memory.h"
#include "bam_context.h"
#include "bam_string.h"
#include "bam_ds.h"

#include "bam_string.inl"
#include "bam_ds.inl"


#endif // guard

// Always allow people to plop an implementation down somewhere,
// regardless of include guards.
#ifdef BAM_IMPLEMENTATION
#include "bam_string.cpp"
#include "bam_context.cpp"
#include "bam_platform.cpp"

#if BAM_NEED_STB_SPRINTF_IMPL
#define STB_SPRINTF_IMPLEMENTATION
#include BAM_STB_SPRINTF_INCLUDE
#endif

#endif

