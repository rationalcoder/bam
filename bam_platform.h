// Only supports Linux and Windows

#ifdef __WIN32
    #define BAM_FORCED_INLINE __forceinline

    #include <malloc.h>
    #define BAM_ALIGNED_ALLOC(_size, _align) _aligned_malloc(_size, _align)
    #define BAM_ALIGNED_FREE(_p) _aligned_free(p)

    #define BAM_CHECK_FMT(_fmt_index, _list_index)
#else
    #define BAM_FORCE_INLINE __attribute__((always_inline)) inline

    #include <stdlib.h>
    #define BAM_ALIGNED_ALLOC(_size, _align) aligned_alloc(_size, _align)
    #define BAM_ALIGNED_FREE(_p) free(_p)

    #define BAM_CHECK_FMT(_fmt_index, _list_index) __attribute__((format (printf, _fmt_index, _list_index)))
#endif


// Placement new b/c <new> is bloated and can complain about exceptions being turned off.
#define bam_placement_new(p) new(bam_new_dummy(), p)

struct bam_new_dummy {};
inline void* operator new(size_t, bam_new_dummy, void* p) { return p; } 
inline void  operator delete(void*, bam_new_dummy, void* p) {} 

namespace bam
{

struct memory_arena;

umm page_size();

auto allocate_arena(const char* tag, umm size, umm max = -1) -> bam::memory_arena;
void free_arena(bam::memory_arena& arena);

u8* expand_arena(bam::memory_arena* arena, u8* at, u8* nextAt, umm alignment);

}

