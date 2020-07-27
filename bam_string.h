namespace bam
{

struct buffer
{
    u8* data;
    umm size;

    buffer() : data(), size() {}
    buffer(void* data, umm size) : data((u8*)data), size(size) {}
    buffer(u8* data, umm size) : data(data), size(size) {}
    buffer(bam::ctor) {}
};

//template <typename T_, typename Size_ = u32>
//struct Array_View
//{
//    T_*   data;
//    Size_ size;
//
//    Array_View(ctor) {}
//    Array_View() : data(nullptr), size(0) {}
//    Array_View(T_* data, Size_ size) : data(data), size(size) {}
//
//    const T_& operator[](Size_ i) const { return data[i]; }
//    T_&       operator[](Size_ i)       { return data[i]; }
//
//    const T_* begin() const { return data; }
//    T_*       begin()       { return data; }
//
//    const T_* end() const { return data + size; }
//    T_*       end()       { return data + size; }
//
//    const T_* cbegin() const { return data; }
//    T_*       cend()   const { return data + size; }
//
//    explicit operator bool() { return data != nullptr; }
//    explicit operator struct string() const;
//};
//
//template <typename T_> using view8  = Array_View<T_, u8>;
//template <typename T_> using view16 = Array_View<T_, u16>;
//template <typename T_> using view32 = Array_View<T_, u32>;
//template <typename T_> using view64 = Array_View<T_, u64>;

//template <typename T_> inline Array_View<T_, u32>
//view_of(T_* data, u32 size) { return Array_View<T_, u32>(data, size); }
//
//template <typename T_, u32 Size_> inline Array_View<T_, u32>
//view_of(T_ (&data)[Size_]) { return Array_View<T_, u32>(data, Size_); }

struct string
{
    char* data;
    umm   size;

    string() : data(), size() {}
    string(const char* s);
    string(char* data, umm size) : data(data), size(size) {}
    string(bam::ctor) {}

    // TODO: Rest of string API

    explicit operator bool() const { return data != nullptr; }
    char operator [] (umm i) const { return data[i]; }
    bool operator == (const bam::string& rhs) const;
    bool operator != (const bam::string& rhs) const;

    bool starts_with(const char* s) const;
    bool starts_with(char c) const;

    bam::string trim() const;

    // :StringHasNul
    umm capacity() const { return size + 1; }

    auto c_str() const -> const char* { return data; }
    auto dup()   const -> bam::string;

    //{ Conversions
    int to_int(int def = 0, bool* ok = nullptr) const;
    s8  to_s8 (s8  def = 0, bool* ok = nullptr) const;
    s16 to_s16(s16 def = 0, bool* ok = nullptr) const;
    s32 to_s32(s32 def = 0, bool* ok = nullptr) const;
    s64 to_s64(s64 def = 0, bool* ok = nullptr) const;

    auto to_unsigned(unsigned def = 0, bool* ok = nullptr) const -> unsigned;
    u8   to_u8      (u8       def = 0, bool* ok = nullptr) const;
    u16  to_u16     (u16      def = 0, bool* ok = nullptr) const;
    u32  to_u32     (u32      def = 0, bool* ok = nullptr) const;
    u64  to_u64     (u64      def = 0, bool* ok = nullptr) const;
    umm  to_usize   (umm      def = 0, bool* ok = nullptr) const;

    auto to_float (float  def = 0, bool* ok = nullptr) const -> float;
    auto to_double(double def = 0, bool* ok = nullptr) const -> double;

    bool to_bool(bool def = false, bool* ok = nullptr) const;
    //}
};


//b32 operator == (const buffer& lhs, const buffer& rhs);
//b32 operator == (const buffer& buffer, char c);
//b32 operator == (const buffer& buffer, const char* str);
//b32 operator != (const buffer& lhs, const buffer& rhs);
//b32 operator != (const buffer& buffer, char c);
//b32 operator != (const buffer& buffer, const char* str);

string fmt(const char* fmt, ...) BAM_CHECK_FMT(1, 2);
string vfmt(const char* fmt, va_list va);

///static string cat(string a, string b);
///static string cat(const char* a, string b);
///static string cat(string a, const char* b);
///static string cat(const char* a, const char* b);
///
///static char* cstr_line(const buffer& b);
///
///static buffer next_line(const buffer& buffer);
///
///// does not include the newline.
///static u32 line_length(const buffer& buffer);
///
///static bool starts_with(const buffer& buffer, const char* prefix);
///static bool starts_with(const buffer& buffer, const char* prefix, u32 n);
///static bool starts_with(const buffer& buffer, const char** prefixes, u32 n);
///template <u32 Size_>
///static bool starts_with(const buffer& buffer, char (&prefixes)[Size_]);
///template <u32 Size_>
///static bool starts_with(const buffer& buffer, const char* (&prefixes)[Size_]);
///
///// TODO: eat_whitespace() ?
///static buffer eat_spaces(const buffer& buffer);
///static buffer eat_spaces_and_tabs(const buffer& buffer);
///
///static string first_word(const buffer& buffer);
///static string next_word(string word, const buffer& buffer);

enum { string_chunk_size = 64 };

struct string_chunk
{
    bam::string_chunk* next;
    char               data[bam::string_chunk_size];
};

// @Incomplete
struct string_builder
{
    bam::allocator     _alloc;

    umm                _nChunks;
    umm                _tailFilled;
    bam::string_chunk* _tail;
    bam::string_chunk  _head;

    bam::free_list     _freeList;

    string_builder(); // uses current allocator
    string_builder(bam::allocator alloc);
    string_builder(const bam::string& s); // uses temp memory

    ~string_builder();

    void append(const char*);
    void append(const bam::string&);
    void append_fmt(const char* fmt, ...) BAM_CHECK_FMT(2, 3);

    umm size() const { return (_nChunks-1) * bam::string_chunk_size + _tailFilled; }

    bam::string str() const; // uses temp memory.
    bam::string dup() const; // uses current allocator.

    void clear();
};


} // namespace bam
