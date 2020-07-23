
// NOTE(blake): the use of char here is mostly a joke; apparently, the int8 types
// don't have to be chars, making certain casting business technically UB...

namespace bam
{

using u8  = unsigned char;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using umm = size_t;

using uptr = uintptr_t;

using s8  = char;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

using b32 = s32;

using flag8  = u8;
using flag16 = u16;
using flag32 = u32;
using flag64 = u64;


// TODO(blake): fast ints, least ints, etc.

using f32 = float;
using f64 = double;

}

// NOTE(bmartin): Repeated out here instead of a namespace to improve error messages.
#if BAM_USE_SHORT_PRIMITIVES
using u8  = unsigned char;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using umm = size_t;

using uptr = uintptr_t;

using s8  = char;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

using b32 = s32;

using flag8  = u8;
using flag16 = u16;
using flag32 = u32;
using flag64 = u64;


// TODO(blake): fast ints, least ints, etc.

using f32 = float;
using f64 = double;
#endif