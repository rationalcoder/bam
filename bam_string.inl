namespace bam
{

inline
string::string(const char* s)
{
    bam_assert(s);

    umm size = 0;
    while (s[size] != '\0')
        size++;

    char* data = (char*)bam_push_bytes(bam::temp_memory(), size + 1);
    BAM_MEMCPY(data, s, size + 1);

    this->data = data;
    this->size = size;
}

inline bool
string::operator == (const bam::string& rhs) const
{
    if (size != rhs.size)
        return false;

    for (umm i = 0; i < size; i++) {
        if (data[i] != rhs.data[i])
            return false;
    }

    return true;
}

inline bool
string::operator != (const bam::string& rhs) const
{
    return !(*this == rhs);
}

inline bool string::starts_with(const char* s) const { return size && strncmp(data, s, strlen(s)) == 0; }
inline bool string::starts_with(char c) const { return size && data[0] == c; }

inline bam::string
string::dup() const
{
    char* space = (char*)bam_allocate_bytes(capacity());
    BAM_MEMCPY(space, data, capacity());

    return bam::string(space, size);
}

inline bam::string
copy_string(const char* text, umm size)
{
    char* space = (char*)bam_allocate_bytes(size + 1);
    BAM_MEMCPY(space, text, size);
    space[size] = '\0';

    return bam::string(space, size);
}

inline bam::string
temp_string(const char* text, umm size)
{
    char* space = (char*)bam_push_bytes(bam::temp_memory(), size + 1);
    BAM_MEMCPY(space, text, size);
    space[size] = '\0';

    return bam::string(space, size);
}

//{ string_builder

// @Incomplete
inline
string_builder::string_builder(bam::allocator alloc)
{
    _alloc      = alloc;
    _nChunks    = 1;
    _tailFilled = 0;
    _head.next  = nullptr;
    _tail       = &_head;
    _freeList.init(alloc);
}

inline
string_builder::string_builder()
{
    bam_placement_new(this) string_builder(*bam::current_allocator());
}

inline
string_builder::string_builder(const bam::string& s)
{
    bam_placement_new(this) string_builder(bam::temp_memory());
    append(s);
}

inline
string_builder::~string_builder()
{
    if (_alloc.has_free()) {
        for (auto* chunk = &_head; chunk; ) {
            auto* next = chunk->next;
            _alloc.free(chunk);

            chunk = next;
        }
    }
}

inline void
string_builder::append(const char* s)
{
    u32 size = (u32)strlen(s);
    u32 progress = 0;

    for (;;) {
        const u32 space = bam::string_chunk_size - _tailFilled;
        const u32 dataRemaining = size - progress;
        const u32 maxFromThisChunk = space < dataRemaining ? space : dataRemaining;
        BAM_MEMCPY(_tail->data + _tailFilled, s + progress, maxFromThisChunk);

        progress += maxFromThisChunk;

        if (progress == size) {
            _tailFilled += maxFromThisChunk;
            break;
        }

        // We've exhausted this chunk, and we need a new one.
        auto* next   = _freeList.get_or_allocate<bam::string_chunk>();
        next->next   = nullptr;
        _tail->next  = next;
        _tail        = next;
        _tailFilled  = 0;
        _nChunks    += 1;
    }
}

inline void
string_builder::append(const bam::string& s)
{
    u32 progress = 0;

    for (;;) {
        const u32 space = bam::string_chunk_size - _tailFilled;
        const u32 dataRemaining = s.size - progress;
        const u32 maxFromThisChunk = space < dataRemaining ? space : dataRemaining;
        BAM_MEMCPY(_tail->data + _tailFilled, s.data + progress, maxFromThisChunk);

        progress += maxFromThisChunk;

        if (progress == s.size) {
            _tailFilled += maxFromThisChunk;
            break;
        }

        // We've exhausted this chunk, and we need a new one.
        auto* next   = _freeList.get_or_allocate<bam::string_chunk>();
        next->next   = nullptr;
        _tail->next  = next;
        _tail        = next;
        _tailFilled  = 0;
        _nChunks    += 1;
    }
}

inline void
string_builder::append_fmt(const char* fmt, ...)
{
    va_list va;
    va_start(va, fmt);

    append(bam::vfmt(fmt, va));

    va_end(va);
}

inline bam::string
string_builder::str() const
{
    umm copyAmount = size();
    umm filled     = 0;

    char* space = (char*)bam_push_bytes(bam::temp_memory(), copyAmount + 1);

    // The last one is the only one that can be partially full.
    for (auto* cur = &_head; cur; cur = cur->next) {
        umm size = cur->next ? bam::string_chunk_size : _tailFilled;
        BAM_MEMCPY(space + filled, cur->data, size);
        filled += bam::string_chunk_size;
    }

    space[copyAmount] = '\0';
    return bam::string(space, copyAmount);
}

inline bam::string
string_builder::dup() const
{
    umm copyAmount = size();
    umm filled     = 0;

    char* space = (char*)bam_allocate_bytes(copyAmount + 1);

    // The last one is the only one that can be partially full.
    for (auto* cur = &_head; cur; cur = cur->next) {
        umm size = cur->next ? bam::string_chunk_size : _tailFilled;
        BAM_MEMCPY(space + filled, cur->data, size);
        filled += bam::string_chunk_size;
    }

    space[copyAmount] = '\0';
    return bam::string(space, copyAmount);
}

inline void
string_builder::clear()
{
    for (auto* cur = _head.next; cur; cur = cur->next) {
        _freeList.add(cur);
    }

    _head.next  = nullptr;
    _tail       = &_head;
    _tailFilled = 0;
    _nChunks    = 1;
}

//}

} // end namespace bam
