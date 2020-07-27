//{ Literals and abbreviations
#ifndef BAM_USE_SHORT_PRIMITIVES
    #define BAM_USE_SHORT_PRIMITIVES 1
#endif

#ifndef BAM_USE_MEM_LITERALS
    #define BAM_USE_MEM_LITERALS 1
#endif
//}

#ifndef BAM_GET_CONTEXT
    #define BAM_DEFAULT_GET_CONTEXT 1
    #define BAM_GET_CONTEXT() bam::get_context()
#else
    #define BAM_DEFAULT_GET_CONTEXT 0
#endif

//{
// Standard library replacements. Rely on include guards when including standard
// headers like <string.h> to make this more maintainable.

#ifndef BAM_MEMCPY
    #include <string.h>
    #define BAM_MEMCPY ::memcpy
#endif

#ifndef BAM_MEMSET
    #include <string.h>
    #define BAM_MEMSET ::memset
#endif

#ifndef BAM_MEMCMP
    #include <string.h>
    #define BAM_MEMCMP ::memcmp
#endif

#ifndef BAM_STRTOLL
    #include <stdlib.h>
    #define BAM_STRTOLL ::strtoll
#endif

#ifndef BAM_STRTOULL
    #include <stdlib.h>
    #define BAM_STRTOULL ::strtoull
#endif

#ifndef BAM_STRTOF
    #include <stdlib.h>
    #define BAM_STRTOF ::strtof
#endif

#ifndef BAM_STRTOD
    #include <stdlib.h>
    #define BAM_STRTOD ::strtod
#endif

//}

#ifndef BAM_STB_SPRINTF_INCLUDE
    #define BAM_STB_SPRINTF_INCLUDE "bam_stb_sprintf.h"
#endif

#ifndef BAM_REPLACE_STDINT
    #include <stdint.h>
#endif

#ifndef BAM_REPLACE_LIMITS
    #include <limits.h>
#endif

#ifndef BAM_REPLACE_ASSERT
    #include <assert.h>
#endif

#ifndef BAM_LOG_ERROR
    #include <stdio.h>
    #define BAM_LOG_ERROR(...) fprintf(stderr, __VA_ARGS__);
#endif

#ifndef BAM_HAVE_STB_SPRINTF
    #define BAM_NEED_STB_SPRINTF_IMPL 1
#endif

