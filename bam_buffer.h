
template <typename T_, typename Size_ = u32>
struct Array_View
{
    T_*   data;
    Size_ size;

    Array_View(ctor) {}
    Array_View() : data(nullptr), size(0) {}
    Array_View(T_* data, Size_ size) : data(data), size(size) {}

    const T_& operator[](Size_ i) const { return data[i]; }
    T_&       operator[](Size_ i)       { return data[i]; }

    const T_* begin() const { return data; }
    T_*       begin()       { return data; }

    const T_* end() const { return data + size; }
    T_*       end()       { return data + size; }

    const T_* cbegin() const { return data; }
    T_*       cend()   const { return data + size; }

    explicit operator bool() { return data != nullptr; }
    explicit operator struct string32() const;
};

template <typename T_> using view8  = Array_View<T_, u8>;
template <typename T_> using view16 = Array_View<T_, u16>;
template <typename T_> using view32 = Array_View<T_, u32>;
template <typename T_> using view64 = Array_View<T_, u64>;

using buffer8  = Array_View<u8, u8>;
using buffer16 = Array_View<u8, u16>;
using buffer32 = Array_View<u8, u32>;
using buffer64 = Array_View<u8, u64>;

// @Duplication: for better error messages and less meta-programming in the Array_View class
// to get limited string32 -> buffer32 conversions. It's not a big deal.
struct string32
{
    char* data;
    u32   size;

    string32(ctor) {}
    string32() : data(nullptr), size(0) {}
    string32(char* data, u32 size) : data(data), size(size) {}

    const char& operator[](u32 i) const { return data[i]; }
    char&       operator[](u32 i)       { return data[i]; }

    const char* begin() const { return data; }
    char*       begin()       { return data; }

    const char* end() const { return data + size; }
    char*       end()       { return data + size; }

    const char* cbegin() const { return data; }
    char*       cend()   const { return data + size; }

    // TODO: explicit conversion to buffers with smaller size reps;
    explicit operator bool()     { return data != nullptr; }
    operator buffer32() { return buffer32((u8*)data, size); }
    operator buffer64() { return buffer64((u8*)data, size); }
};

template <> inline
Array_View<u8, u32>::operator string32() const
{
    return string32((char*)data, size);
}

template <typename T_> inline Array_View<T_, u32>
view_of(T_* data, u32 size) { return Array_View<T_, u32>(data, size); }

template <typename T_, u32 Size_> inline Array_View<T_, u32>
view_of(T_ (&data)[Size_]) { return Array_View<T_, u32>(data, Size_); }




static b32 operator == (buffer32 lhs, buffer32 rhs);
static b32 operator == (buffer32 buffer, char c);
static b32 operator == (buffer32 buffer, const char* str);
static b32 operator != (buffer32 lhs, buffer32 rhs);
static b32 operator != (buffer32 buffer, char c);
static b32 operator != (buffer32 buffer, const char* str);

// stb_printf callback
static char* push_stbsp_temp_storage(char* /*str*/, void* /*userData*/, int len);

static char*    fmt_cstr(const char* fmt, ...);
static string32 fmt(const char* fmt, ...);
static string32 to_str(const char* s);
//static string32 to_str(buffer32 b);
static char*    to_cstr(const string32 b);

static string32 cat(string32 a, string32 b);
static string32 cat(const char* a, string32 b);
static string32 cat(string32 a, const char* b);
static string32 cat(const char* a, const char* b);

static string32 dup(string32 b);
static string32 dup(const char* s);

static char* cstr_line(buffer32 b);

static buffer32 next_line(buffer32 buffer);

// does not include the newline.
static u32 line_length(buffer32 buffer);

static b32 starts_with(buffer32 buffer, const char* prefix);
static b32 starts_with(buffer32 buffer, const char* prefix, u32 n);
static b32 starts_with(buffer32 buffer, const char** prefixes, u32 n);
template <u32 Size_>
static b32 starts_with(buffer32 buffer, char (&prefixes)[Size_]);
template <u32 Size_>
static b32 starts_with(buffer32 buffer, const char* (&prefixes)[Size_]);

// TODO: eat_whitespace() ?
static buffer32 eat_spaces(buffer32 buffer);
static buffer32 eat_spaces_and_tabs(buffer32 buffer);

static string32 first_word(buffer32 buffer);
static string32 next_word(string32 word, buffer32 buffer);


