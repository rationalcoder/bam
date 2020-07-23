#include <windows>

namespace bam
{

extern umm
page_size()
{
    SYSTEM_INFO info = {};
    GetSystemInfo(&info);

    return info.dwPageSize;
}

extern u8*
failed_expand_arena(bam::memory_arena* arena, u8* alignedAt, u8* nextAt, umm alignment)
{
    // Get requested allocation size for better error reporting.
    u8* at            = arena->tailChunk->at;
    umm padding       = (u8*)bam::align_up(at, (s32)alignment) - at;
    umm unalignedSize = nextAt - at - padding;

    BAM_LOG_ERROR("Failed to expand arena '%s': last allocation (size: %u, align: %u)\n",
        arena->tag, unalignedSize, alignment);

    bam_assert(!"Arena expansion failure");
    return nullptr;
}

extern u8*
expand_arena(bam::memory_arena* arena, u8* alignedAt, u8* nextAt, umm alignment)
{
    if (arena->type == bam::arena_type_fixed) {
        if (nextAt > arena->firstChunk.start + arena->maxSize)
            return bam::failed_expand_arena(arena, alignedAt, nextAt, alignment);

        umm spaceNeeded  = nextAt - arena->firstChunk.end;
        umm chunksNeeded = (spaceNeeded / arena->chunkSize);
        if (chunksNeeded * arena->chunkSize < spaceNeeded)
            chunksNeeded++;

        PDWORD oldProtect = 0;
        if (!VirtualProtect(arena->firstChunk.end, spaceNeeded, PAGE_READWRITE, &oldProtect))
            return bam::failed_expand_arena(arena, alignedAt, nextAt, alignment);

        arena->firstChunk.at   = nextAt;
        arena->firstChunk.end += chunksNeeded * arena->chunkSize;

        return alignedAt;
    }
    else {
        bam_assert(alignment <= bam::page_size());

        // NOTE(bmartin): We want to get space for the allocation and _then_ the memory_arena_chunk, since,
        // depending on the alignment/size of the requested allocation, we could waste space otherwise. For example,
        // chunk + allocation(size = 1, alignment = 4_KB) => a whole page with just a chunk struct in it.
        // 
        // If we can get contiguous pages, and the alignedAt was still in the previous chunk, we can slide the allocation
        // down to use the rest of the space in the previous chunk.
        // In practice, however, since the memory-mapped file region grows downward in address space, its very rare to get contiguous pages,
        // even with an appropriate hint, so I'm just not considering it.

        umm allocationSize          = nextAt - alignedAt;
        umm allocationWithChunkSize = (umm)bam::align_up((void*)allocationSize, alignof(bam::memory_arena_chunk))
                                    + sizeof(bam::memory_arena_chunk);

        auto* next = arena->tailChunk->next;
        if (next) {
            // NOTE(bmartin): The arena was reset, and we have a next chunk already allocated, we want to see if it has enough
            // space in it to support this allocation. If so, we can use it, instead. Otherwise, we need to insert a new chunk
            // with enough space _and_ unmap the chunk that wasn't big enough. We want to unmap the chunk that wasn't big enough
            // so we don't run into usages of arenas that cause a bunch of wasted chunks to pile up at the end of the chunk list.
            // For example, if we have 4_KB chunks, and we do: push(4_KB) * 3, reset(), push(8_KB) * 3, push(16_KB) * 3, etc.,
            // we would have three 4_KB and three 8_KB chunks on the end of the list that might not get used.
            //

            if ((umm)(next->end - next->start) >= allocationWithChunkSize) {
                // Existing chunk was big enough.
                arena->tailChunk = next;
                arena->tailChunk->at = arena->tailChunk->start + allocationWithChunkSize;

                return next->start;

            }
            else {
                // Existing chunk wasn't big enough. Unmap it, and replace it with a new one that is big enough.
                //printf("Wasn't big enough\n");

                umm chunksNeeded = (allocationWithChunkSize / arena->chunkSize);
                if (chunksNeeded * arena->chunkSize < allocationWithChunkSize)
                    chunksNeeded++;

                umm sizePaddedToChunkBoundary = chunksNeeded * arena->chunkSize;

                auto* afterNext = next->next;

                // TODO: Insert new chunk without unmapping if VirtualFree fails.
                // Also, there are remapping calls that we could use to avoid a free/alloc.
                // 
                bam_check(VirtualFree(next->start, 0, MEM_RELEASE));

                u8* start = (u8*)VirtualAlloc(nullptr, sizePaddedToChunkBoundary, MEM_COMMIT, PAGE_READWRITE);

                if (start == NULL)
                    return bam::failed_expand_arena(arena, alignedAt, nextAt, alignment);

                auto* newChunk  = (bam::memory_arena_chunk*)(start + allocationSize);
                newChunk->next  = afterNext;
                newChunk->start = start;
                newChunk->end   = start + sizePaddedToChunkBoundary;
                newChunk->at    = start + allocationWithChunkSize;

                arena->tailChunk->next = newChunk;
                arena->tailChunk       = newChunk;

                return newChunk->start;
            }
        }
        else {
            // First time expanding out this far for this arena. We need a new chunk that will be the new tail.

            umm chunksNeeded = (allocationWithChunkSize / arena->chunkSize);
            if (chunksNeeded * arena->chunkSize < allocationWithChunkSize)
                chunksNeeded++;

            umm sizePaddedToChunkBoundary = chunksNeeded * arena->chunkSize;

            u8* start = (u8*)VirtualAlloc(nullptr, sizePaddedToChunkBoundary, MEM_COMMIT, PAGE_READWRITE);

            if (start == NULL)
                return bam::failed_expand_arena(arena, alignedAt, nextAt, alignment);

            auto* newChunk  = (bam::memory_arena_chunk*)(start + allocationSize);
            newChunk->next  = nullptr;
            newChunk->start = start;
            newChunk->end   = start + sizePaddedToChunkBoundary;
            newChunk->at    = start + allocationWithChunkSize;

            arena->tailChunk->next = newChunk;
            arena->tailChunk       = newChunk;

            return newChunk->start;
        }
    }

    bam_invalid_code_path();
    return nullptr;
}

extern bam::memory_arena
allocate_arena(const char* tag, umm chunkSize, umm maxSize)
{
    bam::memory_arena result = {};

    // Non-default max size implies a fixed size arena.
    //
    // NOTE(bmartin): Right now, only fixed-size arenas get a guard page. The assumption is that
    // a guard page per chunk in a dynamic arena would be too wasteful.
    //
    if (maxSize != SIZE_MAX) {
        // NOTE(bmartin): I am being too careful with alignment. The mmap/mprotect calls
        // will do the right thing with non-page-aligned lengths, but I am adjusting the request
        // itself to be aligned just so the page granularity can't be forgotten by callers printing 
        // out arena values or whatever. Since I need the aligned values anyway, I might as well use them.
        umm alignedInitialSize = bam::align_up(chunkSize, bam::page_size());
        umm alignedMaxSize     = bam::align_up(maxSize,   bam::page_size());

        // Tack a guard page onto the end.
        umm maxGuardedSize = alignedMaxSize + bam::page_size();

        // _Reserve_ the full possible range of pages, to be commited as needed.
        u8* start = (u8*)VirtualAlloc(nullptr, maxGuardedSize, MEM_RESERVE, PAGE_READWRITE);

        if (start == NULL) {
            // TODO: logging
            BAM_LOG_ERROR("Failed to allocate arena '%s' (Max Size: %llu): %s\n", 
                    tag, (u64)maxSize, strerror(errno));
            bam_assert(!"Arena allocation failure");

            return result;
        }

        PDWORD oldProtect = 0;
        if (!VirtualProtect(start, alignedInitialSize, PAGE_READWRITE, &oldProtect)) {
            bam_assert(!"Arena allocation failure");

            return result;
        }

        result.type             = bam::arena_type_fixed;
        result.tag              = tag;
        result.expand           = &bam::expand_arena;
        result.user             = nullptr;
        result.firstChunk.next  = nullptr;
        result.firstChunk.start = start;
        result.firstChunk.at    = start;
        result.firstChunk.end   = start + alignedInitialSize;
        result.tailChunk        = &result.firstChunk;
        result.chunkSize        = alignedInitialSize;
        result.maxSize          = alignedMaxSize;
    }
    // Dynamic arena. Expands as needed. Potentially not contiguous.
    else {
        umm alignedInitialSize = bam::align_up(chunkSize, bam::page_size());

        // Just allocate the first chunk.
        u8* start = (u8*)VirtualAlloc(nullptr, alignedInitialSize, MEM_COMMIT, PAGE_READWRITE);
        if (start == NULL) {
            // TODO: logging
            bam_log_error("Failed to allocate arena '%s' (Max Size: %llu): %s\n", 
                    tag, (u64)maxSize, strerror(errno));
            bam_assert(!"Arena allocation failure");
        }

        result.type             = bam::arena_type_dynamic;
        result.tag              = tag;
        result.expand           = &bam::expand_arena;
        result.user             = nullptr;
        result.firstChunk.next  = nullptr;
        result.firstChunk.start = start;
        result.firstChunk.at    = start;
        result.firstChunk.end   = start + alignedInitialSize;
        result.tailChunk        = &result.firstChunk;
        result.chunkSize        = alignedInitialSize;
        result.maxSize          = SIZE_MAX;
    }

    return result;
}

extern void
free_arena(bam::memory_arena& arena)
{
    if (arena.type == bam::arena_type_fixed) {
        // Don't forget to unmap the guard page.
        bam_check(VirtualFree(arena.firstChunk.start, 0, MEM_RELEASE));
    }
    else {
        // Chunk structs after the first one are stored in the chunks themselves, so, just like a
        // normal heap-allocated list deletion, we need to save the 'next' pointer before unmapping.
        auto* next = &arena.firstChunk;
        for (auto* chunk = next; chunk; chunk = next) {
            next = chunk->next;
            // Chunks may be collated, so 'chunkSize' isn't necessarily the size of each chunk.
            bam_check(VirtualFree(chunk->start, 0, MEM_RELEASE));
        }
    }
}

} // namespace bam