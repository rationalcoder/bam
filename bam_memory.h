

namespace bam
{

namespace mem_literals
{
constexpr umm operator ""_B  (unsigned long long value) { return value; }
constexpr umm operator ""_KB (unsigned long long value) { return value * (1 << 10); }
constexpr umm operator ""_MB (unsigned long long value) { return value * (1 << 20); }
constexpr umm operator ""_GB (unsigned long long value) { return value * (1 << 30); }
}

using namespace mem_literals;


struct memory_arena;
using expand_arena_func = u8* (bam::memory_arena* arena, u8* alignedAt, u8* nextAt, umm alignment);

u8* failed_expand_arena(bam::memory_arena* arena, u8* alignedAt, u8* nextAt, umm alignment);

enum memory_arena_type
{
    arena_type_fixed,
    arena_type_dynamic,
};

struct memory_arena_chunk
{
    bam::memory_arena_chunk* next;
    u8*                      start;
    u8*                      at;
    u8*                      end; // last + 1. So in a 4KB chunk (0-4095) this is 4096.
};

struct memory_arena
{
    bam::memory_arena_chunk* tailChunk;
    bam::memory_arena_chunk  firstChunk;

    bam::expand_arena_func*  expand;
    void*                    user;
    const char*              tag;

    bam::memory_arena_type   type;
    umm                      chunkSize;
    umm                      maxSize;

#if 1
    memory_arena() = default;

    // NOTE(bmartin): Arenas can act like by-value handles for the most part, but since we
    // use an embedded first chunk to avoid allocation, we have to step in during assignment,
    // and set our tail to our own embedded first chunk if the tailChunk is the first one.
    memory_arena(const bam::memory_arena& arena) { *this = arena; }
    bam::memory_arena& operator = (const bam::memory_arena& rhs)
    {
        tailChunk  = rhs.tailChunk == &rhs.firstChunk ? &firstChunk : rhs.tailChunk;
        firstChunk = rhs.firstChunk;
        expand     = rhs.expand;
        user       = rhs.user;
        tag        = rhs.tag;
        type       = rhs.type;
        chunkSize  = rhs.chunkSize;
        maxSize    = rhs.maxSize;

        return *this;
    }
#endif
};

struct push_buffer
{
    bam::memory_arena arena;
    u32               count;

    // Removes the need for a sub_allocate_push_buffer() function.
    push_buffer& operator = (const bam::memory_arena& arena) 
    {
        this->arena = arena;
        count = 0;
        return *this;
    }
};

struct chunk16 { u16 size; u8 alignmentOffset; };
struct chunk32 { u32 size; u8 alignmentOffset; };

template <typename T_> inline u8
alignment_offset(T_* header, bam::memory_arena& arena)
{
    return (u8)((u8*)arena.tailChunk->at - (u8*)(header + 1));
}

inline b32   is_aligned(void* address, umm alignPow2) { return ((uptr)address & ((s32)alignPow2-1)) == 0; }
inline void* align_up  (void* address, umm alignPow2) { return (void*)(((uptr)address + (s32)alignPow2-1) & -(s32)alignPow2); }
inline void* align_down(void* address, umm alignPow2) { return (void*)((uptr)address & ~((s32)alignPow2-1)); }

inline b32  is_aligned(uptr address, umm alignPow2) { return bam::is_aligned((void*)address, alignPow2); }
inline uptr align_up  (uptr address, umm alignPow2) { return (uptr)bam::align_up  ((void*)address, alignPow2); }
inline uptr align_down(uptr address, umm alignPow2) { return (uptr)bam::align_down((void*)address, alignPow2); }

inline void*
aligned_offset(void* address, umm offset, s32 alignPow2)
{
    return bam::align_up((u8*)address + offset, alignPow2);
}

inline umm
arena_size(bam::memory_arena& arena, umm elemSize = 1)
{
    umm size = 0;
    for (auto* chunk = &arena.firstChunk; chunk; chunk = chunk->next) {
        size += (chunk->at - chunk->start) / elemSize;
    }

    return size;
} 

struct arena_mark
{
    bam::memory_arena_chunk* chunk;
    u8*                      at;
};

inline bam::arena_mark
mark(bam::memory_arena& arena)
{
    bam::arena_mark result = { arena.tailChunk, arena.tailChunk->at };
    return result;
}

inline void
reset(bam::memory_arena& arena, const bam::arena_mark& mark)
{
    arena.tailChunk     = mark.chunk;
    arena.tailChunk->at = mark.at;
}

inline void
reset(bam::memory_arena& arena, void* to)
{
    bam_assert(arena.type == bam::arena_type_fixed);
    arena.firstChunk.at = (u8*)to;
}

inline void
reset(bam::memory_arena& arena)
{
    arena.tailChunk = &arena.firstChunk;
    arena.firstChunk.at = arena.firstChunk.start;
}

inline void
reset(bam::push_buffer& buffer)
{
    bam::reset(buffer.arena);
    buffer.count = 0;
}

#define bam_push(arena, size, alignment) bam::push(arena, size, alignment)
#define bam_push_bytes(arena, size) bam::push_bytes(arena, size)
#define bam_push_array(arena, n, type) (type*)bam::push(arena, (n)*sizeof(type), alignof(type))
#define bam_push_array_zero(arena, n, type) (type*)bam::push_zero(arena, (n)*sizeof(type), alignof(type))
#define bam_push_array_copy(arena, n, type, data) (type*)bam::push_copy(arena, (n)*sizeof(type), alignof(type), data)
#define bam_push_type(arena, type) ((type*)bam::push(arena, sizeof(type), alignof(type)))
#define bam_push_new(arena, type) bam_placement_new(bam::push(arena, sizeof(type), alignof(type))) type
#define bam_sub_allocate(arena, size, alignment, tag) bam::sub_allocate(arena, size, alignment, tag)
#define bam_pop_type(arena, type) bam::pop(arena, sizeof(type))
#define bam_reset(...) bam::reset(__VA_ARGS__)

inline void*
push(bam::memory_arena& arena, umm size, umm alignment)
{
    u8* at = (u8*)bam::align_up(arena.tailChunk->at, alignment);
    u8* nextAt = at + size;

    if (nextAt > arena.tailChunk->end)
        return arena.expand(&arena, at, nextAt, alignment);

    arena.tailChunk->at = nextAt;
    return at;
}

// Like push(), but without alignment considerations.
inline void*
push_bytes(bam::memory_arena& arena, umm size)
{
    u8* at = arena.tailChunk->at;
    u8* nextAt = at + size;

    if (nextAt > arena.tailChunk->end)
        return arena.expand(&arena, at, nextAt, 1);

    arena.tailChunk->at = nextAt;
    return at;
}

inline void*
push_copy(bam::memory_arena& arena, umm size, umm alignment, const void* data)
{
    void* space = bam::push(arena, size, alignment);
    memcpy(space, data, size);

    return space;
}

inline void*
push_zero(bam::memory_arena& arena, umm size, umm alignment)
{
    void* space = bam::push(arena, size, alignment);
    memset(space, 0, size);

    return space;
}

inline void*
push(bam::push_buffer& buffer, umm size, umm alignment) 
{ buffer.count++; return bam::push(buffer.arena, size, alignment); }

inline void*
push_copy(bam::push_buffer& buffer, umm size, umm alignment, const void* data) 
{ buffer.count++; return bam::push_copy(buffer.arena, size, alignment, data); }

inline void*
push_bytes(bam::push_buffer& buffer, umm size) 
{ buffer.count++; return bam::push_bytes(buffer.arena, size); }


inline bam::memory_arena
sub_allocate(bam::memory_arena& arena, umm size, umm alignment, const char* tag,
             bam::expand_arena_func* expand = &bam::failed_expand_arena)
{
    void* start = bam::push(arena, size, alignment);

    bam::memory_arena result = {};
    result.tailChunk = &result.firstChunk;
	result.firstChunk.next = nullptr;
    result.firstChunk.start = (u8*)start;
    result.firstChunk.at = (u8*)start;
    result.firstChunk.end = (u8*)start + size;
    result.expand = expand;
    result.user = nullptr; // TODO: pass this in?
    result.tag = tag;
    result.type = bam::arena_type_fixed;
    result.chunkSize = size;
    result.maxSize = size;

    return result;
}

// Utilities

struct memory_arena_scope
{
    bam::memory_arena* arena;
    bam::arena_mark    oldMark;

    memory_arena_scope(bam::memory_arena& arena) : arena(&arena) { oldMark = bam::mark(arena); }
    ~memory_arena_scope() { if (arena) bam_reset(*arena, oldMark); }

    memory_arena_scope(bam::memory_arena_scope&& scope) 
        : arena(scope.arena), oldMark(scope.oldMark) 
    {
        scope.arena = nullptr;
    }
};

struct push_buffer_scope
{
    bam::push_buffer* buffer;
    bam::arena_mark   oldMark;

    push_buffer_scope(bam::push_buffer& buffer) : buffer(&buffer) { oldMark = bam::mark(buffer.arena); }
    ~push_buffer_scope() { if (buffer) { bam_reset(buffer->arena, oldMark); buffer->count = 0; } }

    push_buffer_scope(bam::push_buffer_scope&& scope)
        : buffer(scope.buffer), oldMark(scope.oldMark)
    {
        scope.buffer = nullptr;
    }
};

// TODO(bmartin): make_scope() with no args for the arena in the context struct.

inline bam::memory_arena_scope
make_scope(bam::memory_arena& arena) { return bam::memory_arena_scope(arena); }

inline bam::push_buffer_scope
make_scope(bam::push_buffer& buffer) { return bam::push_buffer_scope(buffer); }

#define bam_allocation_scope_impl2(arena, counter) auto _allocationScope##counter = bam::make_scope(arena)
#define bam_allocation_scope_impl1(arena, counter) bam_allocation_scope_impl2(arena, counter)
#define bam_allocation_scope(arena) bam_allocation_scope_impl1(arena, __COUNTER__);

enum allocator_op 
{
    allocator_op_alloc,
    allocator_op_free,
    allocator_op_reset,
    allocator_op_count_
};

enum allocator_flags : flag32
{
    allocator_has_free = 0x1,
};

// NOTE(bmartin): I saw this on Jon Blow's stream.
// C-style allocator to reduce template instantiations.
// Takes an allocator op to improve i-cache locality. @NeedsTesting
//
using allocate_func = void* (bam::allocator_op op, void* udata, void* p, umm size, umm alignment);

inline void* 
arena_allocate(bam::allocator_op op, void* udata, void*, umm size, umm alignment)
{ 
    if (op == bam::allocator_op_alloc)
        return bam::push(*(bam::memory_arena*)udata, size, alignment); 
    else if (op == bam::allocator_op_reset)
        bam::reset(*(bam::memory_arena*)udata);
    // else if (op == bam::allocator_op_free) not implemented for arenas.

    return nullptr;
}

#define bam_allocator_new(allocator, type) bam_placement_new(allocator.allocate(sizeof(type), alignof(type))) type
#define bam_allocator_alloc_type(allocator, type) (type*)allocator.allocate(sizeof(type), alignof(type))

struct allocator
{
    void*                _udata;
    bam::allocate_func*  _alloc;
    flag32               _flags;

    // TODO: This leaves thing uninitialized b/c there is one in the context struct, and we don't
    // want that thing to have a ctor. That member needs to be pulled out b/c this is error prone.
    allocator() {}
    allocator(bam::memory_arena* arena) { _udata = arena; _alloc = &bam::arena_allocate; }
    allocator(bam::memory_arena& arena) { _udata = &arena; _alloc = &bam::arena_allocate; }

    bool has_free() const { return _flags & bam::allocator_has_free; }

    void* allocate(umm size, umm alignment) 
    { return _alloc(bam::allocator_op_alloc, _udata, nullptr, size, alignment); }

    void free(void* p) { _alloc(bam::allocator_op_free, _udata, p, 0, 0); }
    void reset()       { _alloc(bam::allocator_op_reset, _udata, nullptr, 0, 0); }
};

inline bam::memory_arena*
allocator_data_from(bam::memory_arena& arena)
{
    return &arena;
}
inline bam::allocator
allocator_from(bam::memory_arena& arena)
{
    bam::allocator result = {};

    result._udata = &arena;
    result._alloc = &bam::arena_allocate;
    result._flags = 0;

    return result;
}
inline bam::allocator
allocator_from(bam::memory_arena* arena)
{
    return bam::allocator_from(*arena);
}

// @Speed: We can do better for fixed buffer allocations than converting to an arena.
inline bam::memory_arena
allocator_data_from(void* data, umm size)
{
    bam::memory_arena result = {};
    result.tag               = "Fixed Buffer";
    result.user              = nullptr;
    result.expand            = &bam::failed_expand_arena;
    result.chunkSize         = size;
    result.maxSize           = size;
    result.firstChunk.next   = nullptr;
    result.firstChunk.start  = (u8*)data;
    result.firstChunk.at     = (u8*)data;
    result.firstChunk.end    = (u8*)data + size;
    result.tailChunk         = &result.firstChunk;

    return result;
}


inline void*
heap_allocate(bam::allocator_op op, void*, void* p, umm size, umm alignment)
{
    if (op == bam::allocator_op_alloc)
        return BAM_ALIGNED_ALLOC(alignment, size);
    else if (op == bam::allocator_op_free)
        BAM_ALIGNED_FREE(p);

    return nullptr;
}

inline bam::allocator
make_heap_allocator()
{
    bam::allocator result = {};
    result._alloc = &bam::heap_allocate;
    result._udata = nullptr;
    result._flags = bam::allocator_has_free;

    return result;
}

struct allocator_scope
{
    bam::allocator* old;
    bam::allocator  cur;

    allocator_scope(bam::allocator alloc)
        : cur(alloc)
    {
        old = bam::set_allocator(&cur);
    }

    ~allocator_scope() { if (old) bam::set_allocator(old); }

    allocator_scope(allocator_scope&& scope) 
        : old(scope.old), cur(scope.cur)
    { scope.old = nullptr; }
};

#define bam_allocator_scope_impl2(counter, ...)\
_Pragma("GCC diagnostic push")\
_Pragma("GCC diagnostic ignored \"-Wuninitialized\"")\
auto _allocatorScopeData##counter = bam::allocator_data_from(__VA_ARGS__);\
auto _allocatorScope##counter = bam::allocator_scope(bam::allocator_from(_allocatorScopeData##counter));\
_Pragma("GCC diagnostic pop")
#define bam_allocator_scope_impl1(counter, ...) bam_allocator_scope_impl2(counter, __VA_ARGS__)
#define bam_allocator_scope(...) bam_allocator_scope_impl1(__COUNTER__, __VA_ARGS__)


//{ Free list

struct free_list_node
{
    bam::free_list_node* next;
    void*                data;
};

struct free_list
{
    bam::allocator       _alloc;
    // These refer to free and used 'free_list_node's.
    bam::free_list_node  _first;
    bam::free_list_node* _firstFull;
    bam::free_list_node* _firstEmpty;
    
    void init(bam::allocator alloc);
    void add(void* data);
    void* get();

    template <typename T_>
    T_* get_or_allocate();
};

inline void
free_list::init(bam::allocator alloc)
{
    _alloc      = alloc;
    _first.data = nullptr;
    _first.next = &_first;
    _firstFull  = &_first;
    _firstEmpty = &_first;
}

inline void
free_list::add(void* data)
{
    // Reuse old free-list nodes.
    if (!_firstEmpty->data) {
        _firstEmpty->data = data;
        _firstEmpty = _firstEmpty->next;
        return;
    }

    // No old nodes left. Insert a new node. Right after the
    // first used node is the most straightforward.
    auto* node = bam_allocator_alloc_type(_alloc, bam::free_list_node);
    node->data = data;
    node->next = _firstFull->next;
    _firstFull->next = node;
}

inline void*
free_list::get()
{
    if (!_firstFull->data)
        return nullptr;

    void* result = _firstFull->data;
    _firstFull->data = nullptr;
    _firstFull = _firstFull->next;

    return result;
}

template <typename T_> inline T_*
free_list::get_or_allocate()
{
    void* data = get();
    return data ? static_cast<T_*>(data) 
                : bam_allocator_alloc_type(_alloc, T_);
}

//}

// Simple, tiny list stuff for API boundaries
#define bam_list_push(list, p) (((p)->next = (list)), p)

// Header stuff for things like render commands

template <typename T_> inline T_*
next_header(T_* headerAddress, u32 offset)
{
    return (T_*)bam::aligned_offset(headerAddress + 1, offset, alignof(T_));
}

template <typename T_> inline void*
payload_after(T_* headerAddress)
{
    return headerAddress + 1;
}

template <typename T_> inline void*
payload_after_alignment(T_* header)
{
    return (u8*)header + sizeof(T_) + header->alignmentOffset;
}

#define for_each_header_impl2(type, headerVar, pushBuffer, counter)\
u32 _i##counter = 0;\
for (type* headerVar = (type*)pushBuffer.arena.start;\
     _i##counter < pushBuffer.count; _i##counter++, headerVar = next_header(headerVar, headerVar->size))

#define for_each_header_impl1(type, headerVar, pushBuffer, counter)\
for_each_header_impl2(type, headerVar, pushBuffer, counter)

#define for_each_header(type, headerVar, pushBuffer)\
for_each_header_impl1(type, headerVar, pushBuffer, __COUNTER__)

} // namespace bam

#if BAM_USE_MEM_LITERALS
using namespace bam::mem_literals;
#endif
