
/**
 * DOC: B+Tree basics
 *
 * A B+Tree is a data structure for looking up arbitrary (currently allowing
 * size_t, u32, u64 and 2 * u64) keys into pointers. The data structure
 * is described at http://en.wikipedia.org/wiki/B-tree, we currently do not
 * use binary search to find the key on lookups.
 *
 * Each B+Tree consists of a head, that contains bookkeeping information and
 * a variable number (starting with zero) nodes. Each node contains the keys
 * and pointers to sub-nodes, or, for leaf nodes, the keys and values for the
 * tree entries.
 *
 * Each node in this implementation has the following layout:
 * [key1, key2, ..., keyN] [val1, val2, ..., valN]
 *
 * Each key here is an array of words(size_t), geo->no_words in total. The
 * number of keys and values (N) is geo->no_pairs.
 */

/**
 * struct btree_head - btree head
 *
 * @node: the first node in the tree
 * @mempool: mempool used for node allocations
 * @height: current of the tree
 */
struct btree_head
{
	size_t *node;
	int height;
};

/* btree geometry */
struct btree_geo;

/**
 * btree_init - initialise a btree
 *
 * @head: the btree head to initialise
 *
 * This function allocates the memory pool that the
 * btree needs. Returns zero or a negative error code
 * (-%ENOMEM) when memory allocation fails.
 *
 */
int btree_init(struct btree_head *head);

/**
 * btree_destroy - destroy mempool
 *
 * @head: the btree head to destroy
 *
 * This function destroys the internal memory pool, use only
 * when using btree_init(), not with btree_init_mempool().
 */
void btree_destroy(struct btree_head *head);

/**
 * btree_lookup - look up a key in the btree
 *
 * @head: the btree to look in
 * @geo: the btree geometry
 * @key: the key to look up
 *
 * This function returns the value for the given key, or %NULL.
 */
void *btree_lookup(
    struct btree_head *head,
    struct btree_geo  *geo,
    size_t            *key);

/**
 * btree_insert - insert an entry into the btree
 *
 * @head: the btree to add to
 * @geo: the btree geometry
 * @key: the key to add (must not already be present)
 * @val: the value to add (must not be %NULL)
 * @gfp: allocation flags for node allocations
 *
 * This function returns 0 if the item could be added, or an
 * error code if it failed (may fail due to memory pressure).
 */
int btree_insert(
    struct btree_head *head,
    struct btree_geo  *geo,
    size_t            *key,
    void              *val);

/**
 * btree_update - update an entry in the btree
 *
 * @head: the btree to update
 * @geo: the btree geometry
 * @key: the key to update
 * @val: the value to change it to (must not be %NULL)
 *
 * This function returns 0 if the update was successful, or
 * -%ENOENT if the key could not be found.
 */
int btree_update(
    struct btree_head *head,
    struct btree_geo  *geo,
    size_t            *key,
    void              *val);

/**
 * btree_remove - remove an entry from the btree
 *
 * @head: the btree to update
 * @geo: the btree geometry
 * @key: the key to remove
 *
 * This function returns the removed entry, or %NULL if the key
 * could not be found.
 */
void *btree_remove(
    struct btree_head *head,
    struct btree_geo  *geo,
    size_t            *key);

/**
 * btree_merge - merge two btrees
 *
 * @target: the tree that gets all the entries
 * @victim: the tree that gets merged into @target
 * @geo: the btree geometry
 * @gfp: allocation flags
 *
 * The two trees @target and @victim may not contain the same keys,
 * that is a bug and triggers a BUG(). This function returns zero
 * if the trees were merged successfully, and may return a failure
 * when memory allocation fails, in which case both trees might have
 * been partially merged, i.e. some entries have been moved from
 * @victim to @target.
 */
int btree_merge(
    struct btree_head *target,
    struct btree_head *victim,
    struct btree_geo  *geo);

/**
 * btree_last - get last entry in btree
 *
 * @head: btree head
 * @geo: btree geometry
 * @key: last key
 *
 * Returns the last entry in the btree, and sets @key to the key
 * of that entry; returns NULL if the tree is empty, in that case
 * key is not changed.
 */
void *btree_last(
    struct btree_head *head,
    struct btree_geo  *geo,
	size_t            *key);

/**
 * btree_get_prev - get previous entry
 *
 * @head: btree head
 * @geo: btree geometry
 * @key: pointer to key
 *
 * The function returns the next item right before the value pointed to by
 * @key, and updates @key with its key, or returns %NULL when there is no
 * entry with a key smaller than the given key.
 */
void *btree_get_prev(
    struct btree_head *head,
    struct btree_geo  *geo,
    size_t            *key);

typedef void (btree_visitor_fn)(void *elem, size_t opaque, size_t *key, size_t index, void *func2);

/* internal use, use btree_visitor{l,32,64,128} */
size_t btree_visitor(
    struct btree_head *head,
    struct btree_geo  *geo,
    size_t            opaque,
	btree_visitor_fn  *func,
    void              *func2);

/* internal use, use btree_grim_visitor{l,32,64,128} */
size_t btree_grim_visitor(
    struct btree_head *head,
    struct btree_geo  *geo,
	size_t            opaque,
	btree_visitor_fn  *func,
	void              *func2);

