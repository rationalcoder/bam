#include <type_traits>

namespace bam
{

#define bam_invalid_code_path() assert(!"Invalid code path.")
#define bam_not_implemented() assert(!"Not Implemented.")
#define bam_static_not_implemented() static_assert(false, "Not Implemented.")
#define bam_array_size(arr) (sizeof(arr)/sizeof((arr)[0]))

// @Hack
#define bam_rare_assert assert
#define bam_assert assert
#define bam_check(cond) if (!(cond)) { bam_assert(cond); }

#define bam_field_count(type, field) (offsetof(type, field)/sizeof(field))
#define bam_assert_first_field(type, field)\
static_assert(offsetof(type, field) == 0, "Field '" #field "' must come first in '" #type "'")

enum ctor { uninitialized };

template <typename To_, typename From_> inline To_
down_cast(From_ from)
{
    To_ result = (To_)from;
    //bam_assert(result == from);

    return result;
}

template <typename To_> inline To_
down_cast(void* from)
{
    To_ result = (To_)(uptr)from;
    bam_assert((uptr)result == (uptr)from);

    return result;
}

inline f32
map_bilateral(s32 val, u32 range)
{
    return val/(range/2.0f);
}

// NOTE(bmartin): manual overloading to handle ambiguity with type deduction when literals are passed.
inline u8  right_rotate_value(u8&  value, u8  min, u8  max, u8  step = 1) { return value == max ? min : value + step; }
inline u16 right_rotate_value(u16& value, u16 min, u16 max, u16 step = 1) { return value == max ? min : value + step; }
inline u32 right_rotate_value(u32& value, u32 min, u32 max, u32 step = 1) { return value == max ? min : value + step; }
inline u64 right_rotate_value(u64& value, u64 min, u64 max, u64 step = 1) { return value == max ? min : value + step; }


// move and forward. To get them from std, you have to include <utility>, and that thing is huge...

template <typename T_> constexpr typename std::remove_reference<T_>::type&&
move(T_&& t) noexcept { return static_cast<typename std::remove_reference<T_>::type&&>(t); }

template <typename T_> constexpr T_&&
forward(typename std::remove_reference<T_>::type& t) noexcept { return static_cast<T_&&>(t); }

template <typename T_> constexpr T_&&
forward(typename std::remove_reference<T_>::type&& t) noexcept { return static_cast<T_&&>(t); }

template <typename T_> BAM_FORCE_INLINE void
swap(T_& a, T_& b)
{
    T_ temp = bam::move(a);
    a = bam::move(b);
    b = bam::move(temp);
}

}

