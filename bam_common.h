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
inline void right_rotate_value(u8&  value, u8  min, u8  max, u8  step = 1) { value = value == max ? min : value + step; }
inline void right_rotate_value(u16& value, u16 min, u16 max, u16 step = 1) { value = value == max ? min : value + step; }
inline void right_rotate_value(u32& value, u32 min, u32 max, u32 step = 1) { value = value == max ? min : value + step; }
inline void right_rotate_value(u64& value, u64 min, u64 max, u64 step = 1) { value = value == max ? min : value + step; }

}

