namespace bam
{

#if BAM_DEFAULT_GET_CONTEXT
thread_local bam::context g_ctx;
#endif

void
init_context(bam::context& ctx)
{
    using namespace bam::mem_literals;

    ctx.perm  = bam::allocate_arena("Permanent", 4_KB);
    ctx.temp  = bam::allocate_arena("Temp",      4_KB, 32_KB);
    ctx.frame = bam::allocate_arena("Frame",     4_KB, 32_KB);

    ctx.defaultAllocator = bam::allocator_from(ctx.perm);
    ctx.allocator        = &ctx.defaultAllocator;
}

} // namespace bam
