namespace bam
{

// { blist
template <typename T_, u32 BucketSize_> inline
blist<T_, BucketSize_>::blist(bam::memory_arena& arena)
    : _size(0)
{
    bucket_type* first = bam_push_type(arena, bucket_type);

    _arena = &arena;
    _head  = first;
    _tail  = first;
    _next  = first->array();
}

template <typename T_, u32 BucketSize_> inline T_*
blist<T_, BucketSize_>::add_forget()
{
    _size++;
    if (_next != _tail->end())
        return (T_*)_next++;

    bucket_type* newTail = bam_push_type(*_arena, bucket_type);
    _tail->next = newTail;
    _tail       = newTail;
    _next       = newTail->array() + 1;

    return (T_*)newTail->array();
}

template <typename T_, u32 BucketSize_> inline T_*
blist<T_, BucketSize_>::add_default()
{
    _size++;
    if (_next != _tail->end())
        return bam_placement_new(_next++) T_;

    bucket_type* newTail = bam_push_type(*_arena, bucket_type);
    _tail->next = newTail;
    _tail       = newTail;
    _next       = newTail->array() + 1;

    return bam_placement_new(newTail->array()) T_;
}

template <typename T_, u32 BucketSize_> inline T_*
blist<T_, BucketSize_>::add(T_& val)
{
    _size++;
    if (_next != _tail->end())
        return bam_placement_new(_next++) T_(val);

    bucket_type* newTail = bam_push_type(*_arena, bucket_type);
    _tail->next = newTail;
    _tail       = newTail;
    _next       = newTail->array() + 1;

    return bam_placement_new(newTail->array()) T_(val);
}

template <typename T_, u32 BucketSize_> inline T_*
blist<T_, BucketSize_>::add(T_&& val)
{
    _size++;
    if (_next != _tail->end())
        return bam_placement_new(_next++) T_(bam::move(val));

    bucket_type* newTail = bam_push_type(*_arena, bucket_type);
    _tail->next = newTail;
    _tail       = newTail;
    _next       = newTail->array() + 1;

    return bam_placement_new(newTail->array()) T_(bam::move(val));
}

//}

//{ big_array

template <typename T_> inline
big_array<T_>::~big_array()
{
    clear();
}

template <typename T_> inline bool
big_array<T_>::reserve(umm n)
{
    T_* result = bam::expand_big_array(_data, &_capacityInBytes, n * sizeof(T_));
    if (!result) return false;

    _data = result;
    return true;
}

template <typename T_> inline void
big_array<T_>::clear()
{
    bam::free_big_array(_data, _capacityInBytes);
    _data = nullptr;
    _size = 0;
    _capacity = 0;
    _capacityInBytes = 0;
}

template <typename T_> inline T_*
big_array<T_>::_expand_if_necessary()
{
    if (_size == _capacity) {
        T_* data = (T_*)bam::expand_big_array(_data, &_capacityInBytes, _capacityInBytes + 4096);
        if (!data) {
            bam_assert("allocation failure");
            return nullptr;
        }

        _capacity = _capacityInBytes / sizeof(T_);
        _data = data;
    }

    return &_data[_size++];
}

template <typename T_> inline T_*
big_array<T_>::add_forget()
{
    return _expand_if_necessary();
}

template <typename T_> inline T_*
big_array<T_>::add_default()
{
    T_* result = _expand_if_necessary();
    if (!result) return nullptr;

    bam_placement_new(result) T_();
    return result;
}

template <typename T_> inline T_*
big_array<T_>::add(T_& val)
{
    T_* result = _expand_if_necessary();
    if (!result) return nullptr;

    bam_placement_new(result) T_(val);
    return result;
}

template <typename T_> inline T_*
big_array<T_>::add(T_&& val)
{
    T_* result = _expand_if_necessary();
    if (!result) return nullptr;

    bam_placement_new(result) T_(bam::move(val));
    return result;
}

//}

template <typename T_> inline T_*
flatten(const bam::blist<T_>& list)
{
    auto size = list.size();
    if (size == 0) return nullptr;

    auto* space = bam_allocate_array(size, T_);

    umm i = 0;
    for (T_& val : list) {
        bam_placement_new(space + i) T_(val);
        i++;
    }

    return space;
}


}
