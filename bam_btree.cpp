
/*
 * Simple In-memory B+Tree
 *
 * License is GPLv2
 *
 * Copyright (c) 2007-2008 Joern Engel <joern@purestorage.com>
 * Bits and pieces stolen from Peter Zijlstra's code, which is
 * Copyright 2007, Red Hat Inc. Peter Zijlstra
 * GPLv2
 *
 * A relatively simple B+Tree implementation. I have written it as a learning
 * exercise to understand how B+Trees work. Turned out to be useful as well.
 *
 * B+Trees can be used similar to Linux radix trees (which don't have anything
 * in common with textbook radix trees, beware). Prerequisite for them working
 * well is that access to a random tree node is much faster than a large number
 * of operations within each node.
 *
 * Disks have fulfilled the prerequisite for a long time. More recently DRAM
 * has gained similar properties, as memory access times, when measured in cpu
 * cycles, have increased. Cacheline sizes have increased as well, which also
 * helps B+Trees.
 *
 * Compared to radix trees, B+Trees are more efficient when dealing with a
 * sparsely populated address space.  Between 25% and 50% of the memory is
 * occupied with valid pointers. When densely populated, radix trees contain
 * ~98% pointers - hard to beat. Very sparse radix trees contain only ~2%
 * pointers.
 *
 * This particular implementation stores pointers identified by a long value.
 * Storing NULL pointers is illegal, lookup will return NULL when no entry
 * was found.
 *
 * A trick was used that is not commonly found in textbooks. The lowest
 * values are to the right, not to the left. All used slots within a node
 * are on the left, all unused slots contain NUL values. Most operations
 * simply loop once over all slots and terminate on the first NUL.
 */

// XXX
#define BAM_L1_CACHE_BYTES 64

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define NODESIZE MAX(BAM_L1_CACHE_BYTES, 128)
struct btree_geo
{
    int keylen;
    int no_pairs;
    int no_words;
};
//struct btree_geo btree_geo32 = {
//    .keylen = 1,
//    .no_pairs = NODESIZE / sizeof(size_t) / 2,
//    .no_words = NODESIZE / sizeof(size_t) / 2,
//};
//
#define WORDS_PER_U64 (8 / sizeof(size_t))
//struct btree_geo btree_geo64 = {
//    .keylen = WORDS_PER_U64,
//    .no_pairs = NODESIZE / sizeof(size_t) / (1 + WORDS_PER_U64),
//    .no_words = WORDS_PER_U64 * (NODESIZE / sizeof(size_t) / (1 + WORDS_PER_U64)),
//};
//
//struct btree_geo btree_geo128 = {
//    .keylen = 2 * WORDS_PER_U64,
//    .no_pairs = NODESIZE / sizeof(size_t) / (1 + 2 * WORDS_PER_U64),
//    .no_words = 2 * WORDS_PER_U64 * (NODESIZE / sizeof(size_t) / (1 + 2 * WORDS_PER_U64)),
//};

#define MAX_KEYLEN (2 * WORDS_PER_U64)

static size_t *btree_node_alloc(struct btree_head *head)
{
    return NULL;
}

static void btree_node_free(struct btree_head *head, size_t* node)
{
}

static int wordcmp(const size_t *l1, const size_t *l2, size_t n)
{
    size_t i;
    for (i = 0; i < n; i++) {
        if (l1[i] < l2[i])
            return -1;
        if (l1[i] > l2[i])
            return 1;
    }
    return 0;
}
static size_t *wordcpy(size_t *dest, const size_t *src, size_t n)
{
    size_t i;
    for (i = 0; i < n; i++)
        dest[i] = src[i];
    return dest;
}
static size_t *longset(size_t *s, size_t c, size_t n)
{
    size_t i;
    for (i = 0; i < n; i++)
        s[i] = c;
    return s;
}
static void dec_key(struct btree_geo *geo, size_t *key)
{
    size_t val;
    int i;
    for (i = geo->keylen - 1; i >= 0; i--) {
        val = key[i];
        key[i] = val - 1;
        if (val)
            break;
    }
}

static size_t *btree_node_key(struct btree_geo *geo, size_t *node, int n)
{
    return &node[n * geo->keylen];
}
static void *btree_leaf_value(struct btree_geo *geo, size_t *node, int n)
{
    return (void *)node[geo->no_words + n];
}
static size_t *btree_node_value(struct btree_geo *geo, size_t *node, int n)
{
    return (size_t *)node[geo->no_words + n];
}
static void setkey(struct btree_geo *geo, size_t *node, int n,
           size_t *key)
{
    wordcpy(btree_node_key(geo, node, n), key, geo->keylen);
}
static void setval(struct btree_geo *geo, size_t *node, int n,
           void *val)
{
    node[geo->no_words + n] = (size_t) val;
}
static void clearpair(struct btree_geo *geo, size_t *node, int n)
{
    longset(btree_node_key(geo, node, n), 0, geo->keylen);
    node[geo->no_words + n] = 0;
}
static inline void __btree_init(struct btree_head *head)
{
    head->node = NULL;
    head->height = 0;
}
int btree_init(struct btree_head *head)
{
    __btree_init(head);
    return 0;
}

void btree_destroy(struct btree_head *head)
{
    btree_node_free(head, head->node);
    __btree_init(head);
}

void *btree_last(struct btree_head *head, struct btree_geo *geo,
         size_t *key)
{
    int height = head->height;
    size_t *node = head->node;
    if (height == 0)
        return NULL;
    for ( ; height > 1; height--)
        node = btree_node_value(geo, node, 0);
    wordcpy(key, btree_node_key(geo, node, 0), geo->keylen);
    return btree_leaf_value(geo, node, 0);
}

static int keycmp(struct btree_geo *geo, size_t *node, int pos,
          size_t *key)
{
    return wordcmp(btree_node_key(geo, node, pos), key, geo->keylen);
}
static int keyzero(struct btree_geo *geo, size_t *key)
{
    int i;
    for (i = 0; i < geo->keylen; i++)
        if (key[i])
            return 0;
    return 1;
}
void *btree_lookup(struct btree_head *head, struct btree_geo *geo,
        size_t *key)
{
    int i, height = head->height;
    size_t *node = head->node;
    if (height == 0)
        return NULL;
    for ( ; height > 1; height--) {
        for (i = 0; i < geo->no_pairs; i++)
            if (keycmp(geo, node, i, key) <= 0)
                break;
        if (i == geo->no_pairs)
            return NULL;
        node = btree_node_value(geo, node, i);
        if (!node)
            return NULL;
    }
    if (!node)
        return NULL;
    for (i = 0; i < geo->no_pairs; i++)
        if (keycmp(geo, node, i, key) == 0)
            return btree_leaf_value(geo, node, i);
    return NULL;
}

int btree_update(struct btree_head *head, struct btree_geo *geo,
         size_t *key, void *val)
{
    int i, height = head->height;
    size_t *node = head->node;
    if (height == 0)
        return -1;
    for ( ; height > 1; height--) {
        for (i = 0; i < geo->no_pairs; i++)
            if (keycmp(geo, node, i, key) <= 0)
                break;
        if (i == geo->no_pairs)
            return -1;
        node = btree_node_value(geo, node, i);
        if (!node)
            return -1;
    }
    if (!node)
        return -1;
    for (i = 0; i < geo->no_pairs; i++)
        if (keycmp(geo, node, i, key) == 0) {
            setval(geo, node, i, val);
            return 0;
        }
    return -1;
}

/*
 * Usually this function is quite similar to normal lookup.  But the key of
 * a parent node may be smaller than the smallest key of all its siblings.
 * In such a case we cannot just return NULL, as we have only proven that no
 * key smaller than __key, but larger than this parent key exists.
 * So we set __key to the parent key and retry.  We have to use the smallest
 * such parent key, which is the last parent key we encountered.
 */
void *btree_get_prev(struct btree_head *head, struct btree_geo *geo,
             size_t *__key)
{
    int i, height;
    size_t *node, *oldnode;
    size_t *retry_key = NULL, key[MAX_KEYLEN];
    if (keyzero(geo, __key))
        return NULL;
    if (head->height == 0)
        return NULL;
    wordcpy(key, __key, geo->keylen);
retry:
    dec_key(geo, key);
    node = head->node;
    for (height = head->height ; height > 1; height--) {
        for (i = 0; i < geo->no_pairs; i++)
            if (keycmp(geo, node, i, key) <= 0)
                break;
        if (i == geo->no_pairs)
            goto miss;
        oldnode = node;
        node = btree_node_value(geo, node, i);
        if (!node)
            goto miss;
        retry_key = btree_node_key(geo, oldnode, i);
    }
    if (!node)
        goto miss;
    for (i = 0; i < geo->no_pairs; i++) {
        if (keycmp(geo, node, i, key) <= 0) {
            if (btree_leaf_value(geo, node, i)) {
                wordcpy(__key, btree_node_key(geo, node, i), geo->keylen);
                return btree_leaf_value(geo, node, i);
            } else
                goto miss;
        }
    }
miss:
    if (retry_key) {
        wordcpy(key, retry_key, geo->keylen);
        retry_key = NULL;
        goto retry;
    }
    return NULL;
}

static int getpos(struct btree_geo *geo, size_t *node,
        size_t *key)
{
    int i;
    for (i = 0; i < geo->no_pairs; i++) {
        if (keycmp(geo, node, i, key) <= 0)
            break;
    }
    return i;
}
static int getfill(struct btree_geo *geo, size_t *node, int start)
{
    int i;
    for (i = start; i < geo->no_pairs; i++)
        if (!btree_node_value(geo, node, i))
            break;
    return i;
}
/*
 * locate the correct leaf node in the btree
 */
static size_t *find_level(struct btree_head *head, struct btree_geo *geo,
        size_t *key, int level)
{
    size_t *node = head->node;
    int i, height;
    for (height = head->height; height > level; height--) {
        for (i = 0; i < geo->no_pairs; i++)
            if (keycmp(geo, node, i, key) <= 0)
                break;
        if ((i == geo->no_pairs) || !btree_node_value(geo, node, i)) {
            /* right-most key is too large, update it */
            /* FIXME: If the right-most key on higher levels is
             * always zero, this wouldn't be necessary. */
            i--;
            setkey(geo, node, i, key);
        }
        bam_assert(i >= 0);
        node = btree_node_value(geo, node, i);
    }
    bam_assert(node);
    return node;
}
static int btree_grow(struct btree_head *head, struct btree_geo *geo)
{
    size_t *node;
    int fill;
    node = btree_node_alloc(head);
    if (!node)
        return -1;
    if (head->node) {
        fill = getfill(geo, head->node, 0);
        setkey(geo, node, 0, btree_node_key(geo, head->node, fill - 1));
        setval(geo, node, 0, head->node);
    }
    head->node = node;
    head->height++;
    return 0;
}
static void btree_shrink(struct btree_head *head, struct btree_geo *geo)
{
    size_t *node;
    int fill;
    if (head->height <= 1)
        return;
    node = head->node;
    fill = getfill(geo, node, 0);
    bam_assert(fill <= 1);
    head->node = btree_node_value(geo, node, 0);
    head->height--;
    btree_node_free(head, node);
}
static int btree_insert_level(struct btree_head *head, struct btree_geo *geo,
                  size_t *key, void *val, int level)
{
    size_t *node;
    int i, pos, fill, err;
    bam_assert(val);
    if (head->height < level) {
        err = btree_grow(head, geo);
        if (err)
            return err;
    }
retry:
    node = find_level(head, geo, key, level);
    pos = getpos(geo, node, key);
    fill = getfill(geo, node, pos);
    /* two identical keys are not allowed */
    bam_assert(pos >= fill || keycmp(geo, node, pos, key) != 0);
    if (fill == geo->no_pairs) {
        /* need to split node */
        size_t *new_node;
        new_node = btree_node_alloc(head);
        if (!new_node)
            return -1;
        err = btree_insert_level(head, geo,
                btree_node_key(geo, node, fill / 2 - 1),
                new_node, level + 1);
        if (err) {
            btree_node_free(head, new_node);
            return err;
        }
        for (i = 0; i < fill / 2; i++) {
            setkey(geo, new_node, i, btree_node_key(geo, node, i));
            setval(geo, new_node, i, btree_node_value(geo, node, i));
            setkey(geo, node, i, btree_node_key(geo, node, i + fill / 2));
            setval(geo, node, i, btree_node_value(geo, node, i + fill / 2));
            clearpair(geo, node, i + fill / 2);
        }
        if (fill & 1) {
            setkey(geo, node, i, btree_node_key(geo, node, fill - 1));
            setval(geo, node, i, btree_node_value(geo, node, fill - 1));
            clearpair(geo, node, fill - 1);
        }
        goto retry;
    }
    bam_assert(fill < geo->no_pairs);
    /* shift and insert */
    for (i = fill; i > pos; i--) {
        setkey(geo, node, i, btree_node_key(geo, node, i - 1));
        setval(geo, node, i, btree_node_value(geo, node, i - 1));
    }
    setkey(geo, node, pos, key);
    setval(geo, node, pos, val);
    return 0;
}
int btree_insert(struct btree_head *head, struct btree_geo *geo,
        size_t *key, void *val)
{
    bam_assert(val);
    return btree_insert_level(head, geo, key, val, 1);
}

static void *btree_remove_level(struct btree_head *head, struct btree_geo *geo,
        size_t *key, int level);
static void merge(struct btree_head *head, struct btree_geo *geo, int level,
        size_t *left, int lfill,
        size_t *right, int rfill,
        size_t *parent, int lpos)
{
    int i;
    for (i = 0; i < rfill; i++) {
        /* Move all keys to the left */
        setkey(geo, left, lfill + i, btree_node_key(geo, right, i));
        setval(geo, left, lfill + i, btree_node_value(geo, right, i));
    }
    /* Exchange left and right child in parent */
    setval(geo, parent, lpos, right);
    setval(geo, parent, lpos + 1, left);
    /* Remove left (formerly right) child from parent */
    btree_remove_level(head, geo, btree_node_key(geo, parent, lpos), level + 1);
    btree_node_free(head, right);
}
static void rebalance(struct btree_head *head, struct btree_geo *geo,
        size_t *key, int level, size_t *child, int fill)
{
    size_t *parent, *left = NULL, *right = NULL;
    int i, no_left, no_right;
    if (fill == 0) {
        /* Because we don't steal entries from a neighbour, this case
         * can happen.  Parent node contains a single child, this
         * node, so merging with a sibling never happens.
         */
        btree_remove_level(head, geo, key, level + 1);
        btree_node_free(head, child);
        return;
    }
    parent = find_level(head, geo, key, level + 1);
    i = getpos(geo, parent, key);
    bam_assert(btree_node_value(geo, parent, i) == child);
    if (i > 0) {
        left = btree_node_value(geo, parent, i - 1);
        no_left = getfill(geo, left, 0);
        if (fill + no_left <= geo->no_pairs) {
            merge(head, geo, level,
                    left, no_left,
                    child, fill,
                    parent, i - 1);
            return;
        }
    }
    if (i + 1 < getfill(geo, parent, i)) {
        right = btree_node_value(geo, parent, i + 1);
        no_right = getfill(geo, right, 0);
        if (fill + no_right <= geo->no_pairs) {
            merge(head, geo, level,
                    child, fill,
                    right, no_right,
                    parent, i);
            return;
        }
    }
    /*
     * We could also try to steal one entry from the left or right
     * neighbor.  By not doing so we changed the invariant from
     * "all nodes are at least half full" to "no two neighboring
     * nodes can be merged".  Which means that the average fill of
     * all nodes is still half or better.
     */
}
static void *btree_remove_level(struct btree_head *head, struct btree_geo *geo,
        size_t *key, int level)
{
    size_t *node;
    int i, pos, fill;
    void *ret;
    if (level > head->height) {
        /* we recursed all the way up */
        head->height = 0;
        head->node = NULL;
        return NULL;
    }
    node = find_level(head, geo, key, level);
    pos = getpos(geo, node, key);
    fill = getfill(geo, node, pos);
    if ((level == 1) && (keycmp(geo, node, pos, key) != 0))
        return NULL;
    ret = btree_leaf_value(geo, node, pos);
    /* remove and shift */
    for (i = pos; i < fill - 1; i++) {
        setkey(geo, node, i, btree_node_key(geo, node, i + 1));
        setval(geo, node, i, btree_node_value(geo, node, i + 1));
    }
    clearpair(geo, node, fill - 1);
    if (fill - 1 < geo->no_pairs / 2) {
        if (level < head->height)
            rebalance(head, geo, key, level, node, fill - 1);
        else if (fill - 1 == 1)
            btree_shrink(head, geo);
    }
    return ret;
}
void *btree_remove(struct btree_head *head, struct btree_geo *geo,
        size_t *key)
{
    if (head->height == 0)
        return NULL;
    return btree_remove_level(head, geo, key, 1);
}

int btree_merge(struct btree_head *target, struct btree_head *victim,
        struct btree_geo *geo)
{
    size_t key[MAX_KEYLEN];
    size_t dup[MAX_KEYLEN];
    void *val;
    int err;
    bam_assert(target != victim);
    if (!(target->node)) {
        /* target is empty, just copy fields over */
        target->node = victim->node;
        target->height = victim->height;
        __btree_init(victim);
        return 0;
    }
    /* TODO: This needs some optimizations.  Currently we do three tree
     * walks to remove a single object from the victim.
     */
    for (;;) {
        if (!btree_last(victim, geo, key))
            break;
        val = btree_lookup(victim, geo, key);
        err = btree_insert(target, geo, key, val);
        if (err)
            return err;
        /* We must make a copy of the key, as the original will get
         * mangled inside btree_remove. */
        wordcpy(dup, key, geo->keylen);
        btree_remove(victim, geo, dup);
    }
    return 0;
}

static size_t btree_for_each(struct btree_head *head, struct btree_geo *geo,
                   size_t *node, size_t opaque, btree_visitor_fn* func,
                   void *func2, int reap, int height, size_t count)
{
    int i;
    size_t *child;
    for (i = 0; i < geo->no_pairs; i++) {
        child = btree_node_value(geo, node, i);
        if (!child)
            break;
        if (height > 1)
            count = btree_for_each(head, geo, child, opaque,
                func, func2, reap, height - 1, count);
        else
            func(child, opaque, btree_node_key(geo, node, i), count++, func2);
    }
    if (reap)
        btree_node_free(head, node);

    return count;
}

static void empty(void *, size_t, size_t *, size_t, void *) {}

size_t
btree_grim_visitor(struct btree_head *head, struct btree_geo *geo,
              size_t opaque, btree_visitor_fn* func, void *func2)
{
    size_t count = 0;
    if (!func2)
        func = empty;
    if (head->node)
        count = btree_for_each(head, geo, head->node, opaque, func,
                func2, 1, head->height, 0);
    __btree_init(head);
    return count;
}

