

//{ Temp/Perm allocation macros


template <typename T_> inline T_
bam__deduce_type_from_new_expression(T_*);

#define bam_allocate(size, alignment) BAM_ALLOCATE(size, alignment)
#define bam_allocate_bytes(size) (char*)bam_allocate(size, 1)
#define bam_allocate_array(n, type) (type*)bam_allocate((n)*sizeof(type), alignof(type))
#define bam_allocate_array_zero(n, type) (type*)bam__allocate_zero((n)*sizeof(type), alignof(type))
#define bam_allocate_array_copy(n, type, data) (type*)bam__allocate_copy((n)*sizeof(type), alignof(type), data)
#define bam_allocate_type(type) ((type*)bam_allocate(sizeof(type), alignof(type)))
#define bam_new bam_allocate_new
#define bam_allocate_new(newExpr)\
    ([&] () {\
        using Type = decltype(bam__deduce_type_from_new_expression(new newExpr));\
        return (new (bam_allocate(sizeof(Type), alignof(Type))) newExpr);\
    } ())

#define bam_allocate_new_array(n, newExpr)\
    [&]() {\
        using Type = decltype(bam__deduce_type_from_new_expression(new newExpr));\
        Type* data = (Type*)bam_allocate((n)*sizeof(Type), alignof(Type));\
        for (int i = 0; i < (int)n; i++)\
            new (data + i) newExpr;\
        return data;\
    } ()

inline void*
bam__allocate_zero(umm size, umm alignment)
{
    void* space = bam_allocate(size, alignment);
    BAM_MEMSET(space, 0, size);

    return space;
}

inline void*
bam__allocate_copy(umm size, umm alignment, void* data)
{
    void* space = bam_allocate(size, alignment);
    BAM_MEMCPY(space, data, size);

    return space;
}

#define bam_temp_allocate(size, alignment) BAM_TEMP_ALLOCATE(size, alignment)
#define bam_temp_bytes(size) (char*)bam_temp_allocate(size, 1)
#define bam_temp_array(n, type) (type*)bam_temp_allocate((n)*sizeof(type), alignof(type))
#define bam_temp_array_zero(n, type) (type*)bam__temp_allocate_zero((n)*sizeof(type), alignof(type))
#define bam_temp_array_copy(n, type, data) (type*)bam__temp_allocate_copy((n)*sizeof(type), alignof(type), data)
#define bam_temp_type(type) ((type*)bam_temp_allocate(sizeof(type), alignof(type)))
#define bam_temp_new(newExpr)\
    ([&] () {\
        using Type = decltype(bam__deduce_type_from_new_expression(new newExpr));\
        return (new (bam_temp_allocate(sizeof(Type), alignof(Type))) newExpr);\
    } ())

#define bam_temp_new_array(n, newExpr)\
    [&]() {\
        using Type = decltype(bam__deduce_type_from_new_expression(new newExpr));\
        Type* data = (Type*)bam_temp_allocate((n)*sizeof(Type), alignof(Type));\
        for (int i = 0; i < (int)n; i++)\
            new (data + i) newExpr;\
        return data;\
    } ()

inline void*
bam__temp_allocate_zero(umm size, umm alignment)
{
    void* space = bam_allocate(size, alignment);
    BAM_MEMSET(space, 0, size);

    return space;
}

inline void*
bam__temp_allocate_copy(umm size, umm alignment, void* data)
{
    void* space = bam_allocate(size, alignment);
    BAM_MEMCPY(space, data, size);

    return space;
}


//}


using Expand_Arena_Func = b32 (struct Arena* arena, umm failedRequest);

struct Arena
{
    const char* tag;

    Expand_Arena_Func* expand;
    void* user;

    void* start;
    void* at;
    void* next;

    umm size;
    umm max;
};

struct Push_Buffer
{
    Arena arena;
    u32 count;

    Push_Buffer& operator = (const Arena& arena);
};

inline Push_Buffer&
Push_Buffer::operator = (const Arena& arena)
{
    this->arena = arena;
    this->count = 0;
    return *this;
}

inline b32
is_aligned(void* address, s32 alignPow2)
{
    bool result = ((uptr)address & (alignPow2-1)) == 0;
    return result;
}

inline void*
align_up(void* address, s32 alignPow2)
{
    void* result = (void*)(((uptr)address + alignPow2-1) & -alignPow2);
    return result;
}

inline void*
align_down(void* address, s32 alignPow2)
{
    void* result = (void*)((uptr)address & ~(alignPow2-1));
    return result;
}

inline void*
aligned_offset(void* address, umm offset, s32 alignPow2)
{
    void* result = align_up((u8*)address + offset, alignPow2);
    return result;
}

inline umm align_up(umm address, s32 alignPow2)
{ return (umm)align_up((void*)address, alignPow2); }

inline umm align_down(umm address, s32 alignPow2)
{ return (umm)align_down((void*)address, alignPow2); }

inline umm aligned_offset(umm address, umm offset, s32 alignPow2)
{ return (umm)aligned_offset((void*)address, offset, alignPow2); }


inline umm
arena_size(Arena& arena)
{
    umm result = ((u8*)arena.at - (u8*)arena.start);
    return result;
}

inline umm
arena_size(Arena& arena, umm elemSize)
{
    //umm result = ((u8*)arena.at - (u8*)arena.start) / elemSize;
    umm result = arena_size(arena) / elemSize;
    return result;
}

inline void
reset(Arena& arena)
{
    arena.at = arena.start;
}

inline void
reset(Arena& arena, void* to)
{
    arena.at = to;
}

inline void
reset(Push_Buffer& buffer)
{
    reset(buffer.arena);
    buffer.count = 0;
}

#define push_array(arena, n, type) (type*)push(arena, (n)*sizeof(type), alignof(type))
#define push_array_copy(arena, n, type, data) (type*)push_copy(arena, (n)*sizeof(type), alignof(type), data)
#define push_type(arena, type) ((type*)push(arena, sizeof(type), alignof(type)))
#define push_new(arena, type, ...) (new (push(arena, sizeof(type), alignof(type))) type(__VA_ARGS__))

inline void*
push(Arena& arena, umm size, umm alignment)
{
    u8* at = (u8*)align_up(arena.at, (s32)alignment);
    u8* nextAt = at + size;

    if (nextAt > (u8*)arena.next) {
        if (!arena.expand(&arena, nextAt - (u8*)arena.at))
            return nullptr;
    }

    arena.at = nextAt;
    return at;
}

inline void*
push_bytes(Arena& arena, umm size)
{
    u8* at = (u8*)arena.at;
    u8* nextAt = at + size;

    if (nextAt > (u8*)arena.next) {
        if (!arena.expand(&arena, size))
            return nullptr;
    }

    arena.at = nextAt;
    return at;
}

inline void*
push_zero(Arena& arena, umm size, umm alignment)
{
    void* space = push(arena, size, alignment);
    memset(space, 0, size);

    return space;
}

inline void*
push_copy(Arena& arena, umm size, umm alignment, void* data)
{
    void* space = push(arena, size, alignment);
    memcpy(space, data, size);

    return space;
}

#define sub_allocate(...) sub_allocate_(__VA_ARGS__)

inline Arena
sub_allocate_(Arena& arena, umm size, umm alignment, const char* tag,
             Expand_Arena_Func* expand = BAM_FAILED_EXPAND_ARENA())
{
    void* start = push(arena, size, alignment);

    Arena result;
    result.tag    = tag;
    result.expand = expand;
    result.user   = nullptr; // TODO: pass this in?
    result.start  = start;
    result.at     = start;
    result.next   = (u8*)start + size;
    result.size   = size;
    result.max    = ~(umm)0; // max doesn't really make sense but inf is the most reasonable value.

    return result;
}

#define pop_type(arena, type) pop(arena, sizeof(type))

inline void
pop(Arena& arena, umm size)
{
    (u8*&)arena.at -= size;
}

template <typename T_>
struct Arena_Allocator
{
    Arena* arena = nullptr;

    T_* allocate(umm) { return push_bytes(*arena, sizeof(T_)); }
    // @Hack: these can only be deallocated in order.
    void deallocate(T_*, umm) { pop(*arena, sizeof(T_)); }

    bool operator == (Arena_Allocator<T_> rhs) { return arena == rhs.arena; }
    bool operator != (Arena_Allocator<T_> rhs) { return arena != rhs.arena; }
};

struct Memory_Arena_Scope
{
    Arena* arena;
    void* oldAt;

    Memory_Arena_Scope(Arena* arena) : arena(arena) { oldAt = arena->at; }
    ~Memory_Arena_Scope() { if (arena) reset(*arena, oldAt); }

    Memory_Arena_Scope(Memory_Arena_Scope&& scope)
        : arena(scope.arena), oldAt(scope.oldAt)
    {
        scope.arena = nullptr;
    }
};

// TODO: allocation_scope for both arenas and push buffers? Why different names?

#define arena_scope_impl2(arena, counter) Memory_Arena_Scope _arenaScope##counter{&arena}
#define arena_scope_impl1(arena, counter) arena_scope_impl2(arena, counter)
#define arena_scope(arena) arena_scope_impl1(arena, __COUNTER__);

struct Push_Buffer_Scope
{
    Push_Buffer* buffer;
    void* oldAt;

    Push_Buffer_Scope(Push_Buffer& buffer) : buffer(&buffer) { oldAt = buffer.arena.at; }
    ~Push_Buffer_Scope() { if (buffer) reset(buffer->arena, oldAt); buffer->count = 0; }

    Push_Buffer_Scope(Push_Buffer_Scope&& scope)
        : buffer(scope.buffer), oldAt(scope.oldAt)
    {
        scope.buffer = nullptr;
    }
};

#define push_buffer_scope_impl2(buffer, counter) Push_Buffer_Scope _pushBufferScope##counter{&buffer}
#define push_buffer_scope_impl1(buffer, counter) push_buffer_scope_impl2(buffer, __COUNTER__);
#define push_buffer_scope(buffer) push_buffer_scope_impl1(buffer, __COUNTER__);

// TODO: common2-style allocator

using Allocate_Func = void* (void* data, umm size, umm alignment);

struct Allocator
{
    Allocate_Func* func = nullptr;
    void*          data = nullptr;

    Allocator() = default;
    Allocator(Allocate_Func* func, void* data) : func(func), data(data) {}
};

inline void*
arena_allocate(void* arena, umm size, umm alignment)
{ return push(*(Arena*)(arena), size, alignment); }


// Simple, tiny list stuff for API boundaries
#define list_push(list, p) (((p)->next = (list)), p)


// Header stuff for things like render commands

template <typename T_> inline T_*
next_header(T_* headerAddress, u32 offset)
{
    T_* result = (T_*)aligned_offset(headerAddress + 1, offset, alignof(T_));
    return result;
}

#define for_each_header_impl2(type, headerVar, pushBuffer, counter)\
u32 _i##counter = 0;\
for (type* headerVar = (type*)pushBuffer.arena.start;\
     _i##counter < pushBuffer.count; _i##counter++, headerVar = next_header(headerVar, headerVar->size))

#define for_each_header_impl1(type, headerVar, pushBuffer, counter)\
for_each_header_impl2(type, headerVar, pushBuffer, counter)

#define for_each_header(type, headerVar, pushBuffer)\
for_each_header_impl1(type, headerVar, pushBuffer, __COUNTER__)


template <typename T_> inline void*
payload_after(T_* headerAddress)
{
    return headerAddress + 1;
}

