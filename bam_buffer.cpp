
// NOTE(blake): until operator [] is force inlined, access in here will be buffer.data[i], since
// having to step through an extra function like that in the debugger is stupid.

static b32
operator == (buffer32 lhs, buffer32 rhs)
{ return lhs.size == rhs.size && BAM_MEMCMP(lhs.data, rhs.data, lhs.size) == 0; }

static b32
operator == (buffer32 buffer, char c) { return buffer.size == 1 && buffer.data[0] == c; }

static b32
operator == (buffer32 buffer, const char* str)
{
    if (buffer.size == 0) return *str == '\0';

    for (u32 i = 0; i < buffer.size; i++) {
        u8 c = str[i];
        if (!c || c != buffer.data[i])
            return false;
    }

    return true;
}

static b32 operator != (buffer32 lhs, buffer32 rhs) { return !(lhs == rhs); }
static b32 operator != (buffer32 buffer, char c)          { return !(buffer == c); }
static b32 operator != (buffer32 buffer, const char* str) { return !(buffer == str); }


static char*
push_stbsp_temp_storage(char* /*str*/, void* /*userData*/, int len)
{
    if (len < STB_SPRINTF_MIN) return NULL;

    return bam_temp_bytes(STB_SPRINTF_MIN);
}

static char*
fmt_cstr(const char* fmt, ...)
{
    va_list va;
    va_start(va, fmt);

    char* buffer = push_stbsp_temp_storage(NULL, NULL, STB_SPRINTF_MIN);
    s32   n      = stbsp_vsprintfcb(push_stbsp_temp_storage, nullptr, buffer, fmt, va);

    // NOTE(blake): push an extra byte to be sure we can always assign
    // a NUL at the end of the buffer. This works b/c temp memory is contiguous.
    //
    bam_temp_bytes(1);
    buffer[n] = '\0';

    va_end(va);

    return buffer;
}

static string32
fmt(const char* fmt, ...)
{
    va_list va;
    va_start(va, fmt);

    char* buffer = push_stbsp_temp_storage(NULL, NULL, STB_SPRINTF_MIN);
    s32 written  = stbsp_vsprintfcb(push_stbsp_temp_storage, nullptr, buffer, fmt, va);

    va_end(va);

    return string32(buffer, (u32)written);
}

static string32
to_str(const char* s)
{
    u32 size   = down_cast<u32>(strlen(s));
    char* data = bam_temp_bytes(size);
    BAM_MEMCPY(data, s, size);

    return string32(data, size);
}

//static string32
//to_str(buffer32 b)
//{
//    return string32(b.data, b.size);
//}

static char*
to_cstr(string32 b)
{
    char* bytes = bam_temp_bytes(b.size + 1);
    BAM_MEMCPY(bytes, b.data, b.size);

    bytes[b.size] = '\0';
    return bytes;
}

static string32
cat(string32 a, string32 b)
{
    string32 result(uninitialized);

    u32 size = a.size + b.size;
    result.data = bam_temp_bytes(size);
    result.size = size;

    BAM_MEMCPY(result.data, a.data, a.size);
    BAM_MEMCPY(result.data + a.size, b.data, b.size);

    return result;
}

static string32
cat(const char* a, string32 b)
{
    string32 result(uninitialized);

    u32 aSize = (u32)strlen(a);
    u32 size  = aSize + b.size;
    result.data = bam_temp_bytes(size);
    result.size = size;

    BAM_MEMCPY(result.data, a, aSize);
    BAM_MEMCPY(result.data + aSize, b.data, b.size);

    return result;
}

// @Slow

static string32
cat(string32 a, const char* b) { return cat(a, to_str(b)); }

static string32
cat(const char* a, const char* b) { return cat(to_str(a), to_str(b)); }


static string32
dup(string32 b)
{
    char* data = bam_allocate_bytes(b.size);
    BAM_MEMCPY(data, b.data, b.size);

    return string32(data, b.size);
}

static string32
dup(const char* s)
{
    u32   size = (u32)strlen(s);
    char* data = bam_allocate_bytes(size);
    BAM_MEMCPY(data, s, size);

    return string32(data, size);
}

static char*
cstr_line(buffer32 b)
{
    for (u32 i = 0; i < b.size; i++) {
        if (b.data[i] == '\n') {
            u32 size = i + 1;
            char* s = bam_temp_bytes(size + 1);
            BAM_MEMCPY(s, b.data, size);

            s[size] = '\0';
            return s;
        }
    }

    return nullptr;
}

static buffer32
next_line(buffer32 buffer)
{
    buffer32 result = {};
    for (u32 i = 0; i < buffer.size; i++) {
        if (buffer.data[i] == '\n' && (buffer.size - i > 1)) {
            u32 offset = (i + 1);
            result.data = buffer.data + offset;
            result.size = buffer.size - offset;
            return result;
        }
    }

    return result;
}

static u32 // does not include the newline.
line_length(buffer32 buffer)
{
    for (u32 i = 0; i < buffer.size; i++) {
        if (buffer.data[i] == '\n')
            return i;
    }

    return buffer.size;
}

static b32
starts_with(buffer32 buffer, const char* prefix)
{
    for (u32 i = 0; i < buffer.size; i++) {
        u8 c = prefix[i];
        if (!c || c != buffer.data[i])
            return false;
    }

    return true;
}

static b32
starts_with(buffer32 buffer, const char* prefix, u32 n)
{
    if (buffer.size == 0) return false;
    u8 c = buffer.data[0];

    for (u32 i = 0; i < n; i++) {
        if (c == prefix[i])
            return true;
    }

    return false;
}

static b32
starts_with(buffer32 buffer, const char** prefixes, u32 n)
{
    for (u32 i = 0; i < n; i++) {
        if (starts_with(buffer, prefixes[i]))
            return true;
    }

    return false;
}

template <u32 Size_> static b32
starts_with(buffer32 buffer, char (&prefixes)[Size_])
{
    for (u32 i = 0; i < Size_; i++) {
        if (buffer.data[0] == prefixes[i])
            return true;
    }

    return false;
}

template <u32 Size_> static b32
starts_with(buffer32 buffer, const char* (&prefixes)[Size_])
{
    for (u32 i = 0; i < Size_; i++) {
        if (starts_with(buffer, prefixes[i]))
            return true;
    }

    return false;
}

static buffer32
eat_spaces(buffer32 buffer)
{
    buffer32 result = {};
    for (u32 i = 0; i < buffer.size; i++) {
        if (buffer.data[i] != ' ') {
            result.data = buffer.data + i;
            result.size = buffer.size - i;
            return result;
        }
    }

    return result;
}

static buffer32
eat_spaces_and_tabs(buffer32 buffer)
{
    buffer32 result = {};
    for (u32 i = 0; i < buffer.size; i++) {
        u8 c = buffer.data[i];
        if (c != ' ' && c != '\t') {
            result.data = buffer.data + i;
            result.size = buffer.size - i;
            return result;
        }
    }

    return result;
}

static string32
first_word(buffer32 buffer)
{
    buffer = eat_spaces_and_tabs(buffer);
    for (u32 i = 0; i < buffer.size; i++) {
        char c = buffer.data[i];
        if (c == ' ' || c == '\n' || c == '\t') {
            buffer.size = i;
            return (string32)buffer;
        }
    }

    return (string32)buffer;
}

static string32
next_word(string32 word, buffer32 buffer)
{
    string32 result;
    if (buffer.size < word.size + 1)
        return result;

    result.data = word.data + word.size;
    result.size = buffer.size;
    result = (string32)eat_spaces_and_tabs(result);

    for (u32 i = 1; i < result.size; i++) {
        char c = result.data[i];
        if (c == ' ' || c == '\n' || c == '\t') {
            result.size = i;
            return result;
        }
    }

    return result;
}

