
// NOTE(blake): the use of char here is mostly a joke; apparently, the int8 types
// don't have to be chars, making certain casting business technically UB...

using bu8  = unsigned char;
using bu16 = uint16_t;
using bu32 = uint32_t;
using bu64 = uint64_t;
using bumm = size_t;

using buptr = uintptr_t;

using bs8  = char;
using bs16 = int16_t;
using bs32 = int32_t;
using bs64 = int64_t;

using bb32 = int32_t;

// TODO(blake): fast ints, least ints, etc.

using bf32 = float;
using bf64 = double;

