namespace bam
{

template <typename T_, u32 Size_>
struct bucket
{
    char data[Size_ * sizeof(T_)] __attribute__((aligned (alignof(T_))));
    bucket* next;

    bucket() = default;
    bucket(bucket* next) : next(next) {}

    const T_* array() const { return (T_*)&data; }
    T_*       array()       { return (T_*)&data; }

    const T_* last() const { return (T_*)&data + (Size_-1); }
    T_*       last()       { return (T_*)&data + (Size_-1); }

    const T_* end() const { return (T_*)&data + Size_; }
    T_*       end()       { return (T_*)&data + Size_; }
};

template <typename T_, u32 Size_>
struct bucket_iterator
{
    using bucket_type = bam::bucket<T_, Size_>;

    mutable bucket_type* _bucket;
    mutable T_*          _cur;
    
    bucket_iterator(bucket_type* bucket, T_* cur)
        : _bucket(bucket), _cur(cur)
    {}
    
    bool operator == (const bucket_iterator& rhs) const { return _cur == rhs._cur; }
    bool operator != (const bucket_iterator& rhs) const { return _cur != rhs._cur; }

    const T_& operator  * () const { return *_cur; }
    T_&       operator  * ()       { return *_cur; }

    // FIXME(bmartin): -> deref operator not working.

    const T_* operator -> () const { return _cur; }
    T_*       operator -> ()       { return _cur; }

    // NOTE(bmartin): the weird const stuff is to have const bucket_iterator
    // be a valid const_iterator type.
    const bucket_iterator& operator ++ () const
    {
        if (_cur != _bucket->last()) {
            _cur++;
        }
        else {
            bucket_type* nextBucket = _bucket->next;
            _bucket = nextBucket;
            _cur    = (T_*)nextBucket;
        }

        return *this;
    }

    const bucket_iterator operator ++ (int) const
    {
        bucket_iterator result(_bucket, _cur);
        ++result;
        return result;
    }

    bucket_iterator& operator ++ () 
    { return (bucket_iterator&)++(*(const bucket_iterator*)this); }

    bucket_iterator operator ++ (int) 
    { return (bucket_iterator)(*(const bucket_iterator*)this)++; }
};

// @Incomplete
template <typename T_, u32 BucketSize_ = 16>
struct blist
{
    using bucket_type    = bam::bucket<T_, BucketSize_>;
    using iterator       = bam::bucket_iterator<T_, BucketSize_>;
    using const_iterator = const bam::bucket_iterator<T_, BucketSize_>;

    bucket_type* _head;
    bucket_type* _tail;
    T_*          _next;

    bam::memory_arena* _arena;

    u32 _size;

    explicit blist(bam::ctor) {}
    explicit blist(bam::memory_arena& arena);
    blist() : _head(), _tail(), _next(), _arena(), _size() {}

    void reset(bam::memory_arena& arena) { bam_placement_new(this) blist<T_, BucketSize_>(arena); }
    void clear() { bam_placement_new(this) blist<T_, BucketSize_>(*_arena); }

    const T_& front() const { return *_head->array(); }
    T_&       front()       { return *_head->array(); }

    T_* add_forget();
    T_* add_default();
    T_* add(T_& val);
    T_* add(T_&& val);

    u32 size() const { return _size; }

    const_iterator begin() const { return ((blist<T_, BucketSize_>*)this)->begin(); }
    iterator       begin()       
    { return size() == 0 ? iterator(nullptr, nullptr) : iterator(_head, (T_*)_head); }

    const_iterator end() const { return ((blist<T_, BucketSize_>*)this)->end(); }
    iterator       end()       
    { return size() == 0 ? iterator(nullptr, nullptr) : iterator(_tail, _next); }

    const_iterator cbegin() const { return begin(); }
    const_iterator cend()   const { return end(); }
};

// @Incomplete
template <typename T_>
struct big_array
{
    T_* _data;
    umm _size;
    umm _capacity;
    umm _capacityInBytes;

    big_array() : _data(), _size(), _capacity(), _capacityInBytes() {}
    big_array(const big_array<T_>& rhs) = delete;
    ~big_array();

    const T_& operator [] (umm idx) const { bam_assert(idx<_size); return _data[idx]; }
    T_&       operator [] (umm idx)       { bam_assert(idx<_size); return _data[idx]; }

    bool reserve(umm n);
    void clear();

    const T_* data() const { return _data; }
    T_*       data()       { return _data; }

    const T_& front() const { return *_data; }
    T_&       front()       { return *_data; }

    T_* add_forget();
    T_* add_default();
    T_* add(T_& val);
    T_* add(T_&& val);

    umm size() const { return _size; }

    const T_* begin() const { return _data; }
    T_*       begin()       { return _data; }

    const T_* end() const { return _data + _size; }
    T_*       end()       { return _data + _size; }

    const T_* cbegin() const { return begin(); }
    const T_* cend()   const { return end(); }

    BAM_FORCE_INLINE T_* _expand_if_necessary();
};

} // namespace bam
