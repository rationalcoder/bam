namespace bam
{

// NOTE(blake): until operator [] is force inlined, access in here will be buffer.data[i], since
// having to step through an extra function like that in the debugger is stupid.

//extern b32
//operator == (const buffer& lhs, const buffer& rhs)
//{ return lhs.size == rhs.size && BAM_MEMCMP(lhs.data, rhs.data, lhs.size) == 0; }
//
//extern b32
//operator == (const buffer& buffer, char c) { return buffer.size == 1 && buffer.data[0] == c; }
//
//extern b32
//operator == (const buffer& buffer, const char* str)
//{
//    if (buffer.size == 0) return *str == '\0';
//
//    for (umm i = 0; i < buffer.size; i++) {
//        u8 c = str[i];
//        if (!c || c != buffer.data[i])
//            return false;
//    }
//
//    return true;
//}
//
//extern b32 operator != (const buffer& lhs, const buffer& rhs) { return !(lhs == rhs); }
//extern b32 operator != (const buffer& buffer, char c)          { return !(buffer == c); }
//extern b32 operator != (const buffer& buffer, const char* str) { return !(buffer == str); }


// @Slow

//static char*
//cstr_line(const buffer& b)
//{
//    for (umm i = 0; i < b.size; i++) {
//        if (b.data[i] == '\n') {
//            umm size = i + 1;
//            char* s = bam_temp_bytes(size + 1);
//            BAM_MEMCPY(s, b.data, size);
//
//            s[size] = '\0';
//            return s;
//        }
//    }
//
//    return nullptr;
//}
//
//static buffer
//next_line(const buffer& buffer)
//{
//    buffer result = {};
//    for (umm i = 0; i < buffer.size; i++) {
//        if (buffer.data[i] == '\n' && (buffer.size - i > 1)) {
//            umm offset = (i + 1);
//            result.data = buffer.data + offset;
//            result.size = buffer.size - offset;
//            return result;
//        }
//    }
//
//    return result;
//}
//
//static umm // does not include the newline.
//line_length(const buffer& buffer)
//{
//    for (umm i = 0; i < buffer.size; i++) {
//        if (buffer.data[i] == '\n')
//            return i;
//    }
//
//    return buffer.size;
//}
//
//static b32
//starts_with(const buffer& buffer, const char* prefix)
//{
//    for (umm i = 0; i < buffer.size; i++) {
//        u8 c = prefix[i];
//        if (!c || c != buffer.data[i])
//            return false;
//    }
//
//    return true;
//}
//
//static b32
//starts_with(const buffer& buffer, const char* prefix, umm n)
//{
//    if (buffer.size == 0) return false;
//    u8 c = buffer.data[0];
//
//    for (umm i = 0; i < n; i++) {
//        if (c == prefix[i])
//            return true;
//    }
//
//    return false;
//}
//
//static b32
//starts_with(const buffer& buffer, const char** prefixes, umm n)
//{
//    for (umm i = 0; i < n; i++) {
//        if (starts_with(buffer, prefixes[i]))
//            return true;
//    }
//
//    return false;
//}
//
//template <umm Size_> static b32
//starts_with(const buffer& buffer, char (&prefixes)[Size_])
//{
//    for (umm i = 0; i < Size_; i++) {
//        if (buffer.data[0] == prefixes[i])
//            return true;
//    }
//
//    return false;
//}
//
//template <umm Size_> static b32
//starts_with(const buffer& buffer, const char* (&prefixes)[Size_])
//{
//    for (umm i = 0; i < Size_; i++) {
//        if (starts_with(buffer, prefixes[i]))
//            return true;
//    }
//
//    return false;
//}
//
//static buffer
//eat_spaces(const buffer& buffer)
//{
//    buffer result = {};
//    for (umm i = 0; i < buffer.size; i++) {
//        if (buffer.data[i] != ' ') {
//            result.data = buffer.data + i;
//            result.size = buffer.size - i;
//            return result;
//        }
//    }
//
//    return result;
//}
//
//static buffer
//eat_spaces_and_tabs(const buffer& buffer)
//{
//    buffer result = {};
//    for (umm i = 0; i < buffer.size; i++) {
//        u8 c = buffer.data[i];
//        if (c != ' ' && c != '\t') {
//            result.data = buffer.data + i;
//            result.size = buffer.size - i;
//            return result;
//        }
//    }
//
//    return result;
//}
//
//static string
//first_word(const buffer& buffer)
//{
//    buffer = eat_spaces_and_tabs(buffer);
//    for (umm i = 0; i < buffer.size; i++) {
//        char c = buffer.data[i];
//        if (c == ' ' || c == '\n' || c == '\t') {
//            buffer.size = i;
//            return (string)buffer;
//        }
//    }
//
//    return (string)buffer;
//}

//static string
//next_word(const string& word, const buffer& buffer)
//{
//    string result;
//    if (buffer.size < word.size + 1)
//        return result;
//
//    result.data = word.data + word.size;
//    result.size = buffer.size;
//    result = (string)eat_spaces_and_tabs(result);
//
//    for (umm i = 1; i < result.size; i++) {
//        char c = result.data[i];
//        if (c == ' ' || c == '\n' || c == '\t') {
//            result.size = i;
//            return result;
//        }
//    }
//
//    return result;
//}


inline string
string::trim() const
{
    if (!data)
        return string();

    if (!size) {
        char* nul = (char*)bam_push_bytes(bam::temp_memory(), 1);
        *nul = '\0';
        return string(nul, 0);
    }

    char* left = data;
    while (*left && (*left == ' ' || *left == '\n' || *left == '\t' || *left == '\r'))
        left++;

    char* right = data + size-1;
    while (right >= data && (*right == ' ' || *right == '\n' || *right == '\t' || *right == '\r'))
        right--;

    // Whole string is space.
    if (right < left) {
        char* nul = (char*)bam_push_bytes(bam::temp_memory(), 1);
        *nul = '\0';
        return string(nul, 0);
    }

    umm   trimmedSize = right - left + 1;
    char* trimmedData = (char*)bam_push_bytes(bam::temp_memory(), trimmedSize + 1);
    BAM_MEMCPY(trimmedData, left, trimmedSize);
    trimmedData[trimmedSize] = '\0';

    return string(trimmedData, trimmedSize);
}

//{ Conversions

template <typename T_> static inline T_
_to_signed(const string* s, T_ min, T_ max, T_ def, bool* ok)
{
    errno = 0;
    char* firstInvalid;
    auto val = BAM_STRTOLL(s->data, &firstInvalid, 0);

    if (errno != 0 || firstInvalid == s->data || val < min || val > max) {
        if (ok) *ok = false;
        return def;
    }

    if (ok) *ok = true;
    return (T_)val;
}

template <typename T_> static inline T_
_to_unsigned(const string* s, T_ max, T_ def, bool* ok)
{
    errno = 0;
    char* firstInvalid;
    auto val = BAM_STRTOULL(s->data, &firstInvalid, 0);

    if (errno != 0 || firstInvalid == s->data || val > max) {
        if (ok) *ok = false;
        return def;
    }

    if (ok) *ok = true;
    return (T_)val;
}

int string::to_int(int def, bool* ok) const { return _to_signed<int>(this, INT_MIN,   INT_MAX,   def, ok); }
s8  string::to_s8 (s8  def, bool* ok) const { return _to_signed<s8> (this, INT8_MIN,  INT8_MAX,  def, ok); }
s16 string::to_s16(s16 def, bool* ok) const { return _to_signed<s16>(this, INT16_MIN, INT16_MAX, def, ok); }
s32 string::to_s32(s32 def, bool* ok) const { return _to_signed<s32>(this, INT32_MIN, INT32_MAX, def, ok); }
s64 string::to_s64(s64 def, bool* ok) const { return _to_signed<s64>(this, INT64_MIN, INT64_MAX, def, ok); }

unsigned string::to_unsigned(unsigned def, bool* ok) const { return _to_unsigned<unsigned>(this, UINT_MAX,   def, ok); }
u8       string::to_u8      (u8       def, bool* ok) const { return _to_unsigned<u8>      (this, UINT8_MAX,  def, ok); }
u16      string::to_u16     (u16      def, bool* ok) const { return _to_unsigned<u16>     (this, UINT16_MAX, def, ok); }
u32      string::to_u32     (u32      def, bool* ok) const { return _to_unsigned<u32>     (this, UINT32_MAX, def, ok); }
u64      string::to_u64     (u64      def, bool* ok) const { return _to_unsigned<u64>     (this, UINT64_MAX, def, ok); }
umm      string::to_usize   (umm      def, bool* ok) const { return _to_unsigned<umm>     (this, SIZE_MAX,   def, ok); }

float
string::to_float(float def, bool* ok) const
{
    errno = 0;
    float val = BAM_STRTOF(data, nullptr);
    if (errno != 0) {
        if (ok) *ok = false;
        return def;
    }

    if (ok) *ok = true;
    return val;
}

double
string::to_double(double def, bool* ok) const
{
    errno = 0;
    double val = BAM_STRTOD(data, nullptr);
    if (errno != 0) {
        if (ok) *ok = false;
        return def;
    }

    if (ok) *ok = true;
    return val;
}

bool
string::to_bool(bool def, bool* ok) const
{
    if (size >= 4) {
        if (data[0] == 't' && data[1] == 'r' && data[2] == 'u' && data[3] == 'e') {
            if (ok) *ok = true;
            return true;
        }

        // This is fine even if size == 4 b/c of the nul.
        if (data[0] == 'f' && data[1] == 'a' && data[2] == 'l' && data[3] == 's' && data[4] == 'e') {
            if (ok) *ok = true;
            return false;
        }
    }

    if (ok) *ok = false;
    return def;
}

//}

//{ fmt

extern string
fmt(const char* fmt, ...)
{
    va_list va;
    va_start(va, fmt);

    auto result = vfmt(fmt, va);

    va_end(va);
    return result;
}

extern string
vfmt(const char* fmt, va_list va)
{
    string result(bam::uninitialized);

    static constexpr int kStringChunkSize = STB_SPRINTF_MIN;

    auto& tempArena = bam::temp_memory();

    STBSP_SPRINTFCB* sprintfCb = [](char* buf, void* user, int len) -> char* {
        // We may be done printing right at a chunk boundary, in which case we don't need
        // to allocate another chunk, but we have no way of knowing. We shrink the temp
        // arena to fit whatever the string ended up being anyway, so one excess cheap
        // allocation doesn't matter.
        // IMPORTANT(bmartin): This also makes sure that we always have room to add a nul.
        //
        if (len == kStringChunkSize)
            return (char*)bam_push_bytes(*(bam::memory_arena*)user, kStringChunkSize);

        return nullptr;
    };

    char* buf   = (char*)bam_push_bytes(tempArena, kStringChunkSize);
    result.data = buf;
    // Size of string doesn't include the nul, and stb_sprintf returns the size not including nul,
    // since we are responsible for adding that when we use the callback version.
    // :StringHasNul
    result.size = stbsp_vsprintfcb(sprintfCb, &tempArena, buf, fmt, va);
    result.data[result.size] = '\0';

    // Shrink down to fit the allocated string tightly, including the nul.
    // :StringHasNul
    bam_reset(tempArena, buf + result.capacity());

    return result;
}

//}

} // namespace bam
