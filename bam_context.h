namespace bam
{

struct context
{
    bam::allocator*   allocator;
    bam::allocator    defaultAllocator;
    bam::memory_arena perm;
    bam::memory_arena temp;
    bam::memory_arena frame;
};

#if BAM_DEFAULT_GET_CONTEXT
extern thread_local bam::context g_ctx;
bam::context& get_context() { return g_ctx; }
#else
bam::context& get_context() { return BAM_GET_CONTEXT(); }
#endif

BAM_FORCE_INLINE bam::memory_arena&
perm_memory() { return bam::get_context().perm; }

BAM_FORCE_INLINE bam::memory_arena&
temp_memory() { return bam::get_context().temp; }

void init_context(bam::context& ctx);

inline bam::allocator*
set_allocator(bam::allocator* alloc)
{
    auto& ctx = bam::get_context();
    auto* old = ctx.allocator;
    ctx.allocator = alloc;

    return old;
}

BAM_FORCE_INLINE bam::allocator* current_allocator() { return bam::get_context().allocator; }

//{ Allocation stuff that needs to use the execution context.
#define bam_temp_scope() bam_allocation_scope(bam::get_context().temp)

BAM_FORCE_INLINE void* 
allocate(umm size, umm alignment)
{
    return bam::get_context().allocator->allocate(size, alignment);
}

BAM_FORCE_INLINE void* 
allocate_array_copy(umm size, umm alignment, const void* data)
{
    void* space = bam::allocate(size, alignment);
    BAM_MEMCPY(space, data, size);

    return space;
}

BAM_FORCE_INLINE void* 
allocate_array_zero(umm size, umm alignment)
{
    void* space = bam::allocate(size, alignment);
    BAM_MEMSET(space, 0, size);

    return space;
}

#define bam_allocate(size, alignment) bam::allocate((size), (alignment))
#define bam_allocate_bytes(n) bam::allocate((n), 1)
#define bam_allocate_array(n, type) (type*)bam::allocate((n)*sizeof(type), alignof(type))
#define bam_allocate_array_copy(n, type, data) (type*)bam::allocate_array_copy((n)*sizeof(type), alignof(type), (data))
#define bam_allocate_array_zero(n, type) (type*)bam::allocate_array_zero((n)*sizeof(type), alignof(type))
#define bam_allocate_type(type) ((type*)bam::allocate(sizeof(type), alignof(type)))
// TODO(bmartin): better 'new' syntax (much more complicated to implement).
#define bam_allocate_new(type, ...) bam_placement_new(bam::allocate(sizeof(type), alignof(type))) type
//}


} // namespace bam
