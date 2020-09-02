#include <time.h>
#include <stdlib.h>
#include <random>
#include <unordered_set>
#include <map>
#include <algorithm>
#include <bam.h>

#define BAM_IMPLEMENTATION
#include <bam.h>

int main()
{
    bam::init_context();

    bam_push(bam::perm_memory(), 16_MB, 1);
    bam_reset(bam::perm_memory());

    std::map<size_t, void*> rb_tree;

    btree_head tree;
    btree_init(&tree, bam::perm_memory());

    struct timespec before = {};
    struct timespec after = {};

    //clock_gettime(CLOCK_MONOTONIC_RAW, &before);

    std::vector<size_t> data;
    std::vector<size_t> indices;
    for (size_t i = 1; i <= 10000; i++) {
        data.push_back(i);
    }

    for (size_t i = 0; i < 10000; i++) {
        indices.push_back(i);
    }

    std::random_shuffle(indices.begin(), indices.end());

    umm total;
    umm n;
    umm max;
    u32 diff_us;

    total = 0;
    n     = 0;
    max   = 0;
    clock_gettime(CLOCK_MONOTONIC, &before);
    for (size_t idx : indices) {
        rb_tree[idx] = (void*)data[idx];

        //total += diff_us;
        //n++;
        //if (diff_us > max)
        //    max = diff_us;
    }
    clock_gettime(CLOCK_MONOTONIC, &after);

    diff_us = (u32)(after.tv_sec - before.tv_sec) * 1000000 + (u32)(after.tv_nsec - before.tv_nsec) / 1000;
    printf("I Diff RB: %f us\n", (double)diff_us / 100000);

    total = 0;
    n     = 0;
    max   = 0;
    volatile void* value = 0;
    clock_gettime(CLOCK_MONOTONIC, &before);
    for (size_t idx : indices) {
        value = rb_tree[idx];

        //u32 diff_us = (u32)(after.tv_sec - before.tv_sec) * 1000000 + (u32)(after.tv_nsec - before.tv_nsec) / 1000;
        //total += diff_us;
        //n++;
        //if (diff_us > max)
        //    max = diff_us;
    }
    clock_gettime(CLOCK_MONOTONIC, &after);

    diff_us = (u32)(after.tv_sec - before.tv_sec) * 1000000 + (u32)(after.tv_nsec - before.tv_nsec) / 1000;
    printf("L Diff RB: %f us\n", (double)diff_us / 100000);
    printf("%p\n", value);

    total = 0;
    n     = 0;
    max   = 0;
    clock_gettime(CLOCK_MONOTONIC, &before);
    for (size_t idx : indices) {
        btree_insert(&tree, &btree_geo32, &idx, (void*)data[idx]);

        //total += diff_us;
        //n++;
        //if (diff_us > max)
        //    max = diff_us;
    }
    clock_gettime(CLOCK_MONOTONIC, &after);
    
    diff_us = (u32)(after.tv_sec - before.tv_sec) * 1000000 + (u32)(after.tv_nsec - before.tv_nsec) / 1000;
    printf("I Diff B: %f us\n", (double)diff_us / 100000);

    total = 0;
    n     = 0;
    max   = 0;
    value = 0;
    clock_gettime(CLOCK_MONOTONIC, &before);
    for (size_t idx : indices) {
        
        value = btree_lookup(&tree, &btree_geo32, &idx);

        //total += diff_us;
        //n++;
        //if (diff_us > max)
        //    max = diff_us;
    }
    clock_gettime(CLOCK_MONOTONIC, &after);

    diff_us = (u32)(after.tv_sec - before.tv_sec) * 1000000 + (u32)(after.tv_nsec - before.tv_nsec) / 1000;
    printf("L Diff B: %f us\n", (double)diff_us / 100000);
    printf("%p\n", value);

    size_t key = 0;
    size_t* node = nullptr;
    printf("First: %u\n", (size_t)btree_first(&tree, &btree_geo32, &key, &node));
    printf("Prev: %p\n", btree_leaf_prev(&btree_geo32, node));

    for ( ; node; node = btree_leaf_prev(&btree_geo32, node)) {
        int fill = getfill(&btree_geo32, node, 0);
        for (int i = fill; i > 0; i--) {
            printf("%u\n", (size_t)btree_leaf_value(&btree_geo32, node, i-1));
        }
    }

    int i = 0;
    for (size_t idx : indices) {
        if (i++ == 10000)
            break;

        btree_remove(&tree, &btree_geo32, &idx);
    }

    printf("First: %u\n", (size_t)btree_first(&tree, &btree_geo32, &key, &node));
    printf("Prev: %p\n", btree_leaf_prev(&btree_geo32, node));

    for ( ; node; node = btree_leaf_prev(&btree_geo32, node)) {
        int fill = getfill(&btree_geo32, node, 0);
        for (int i = fill; i > 0; i--) {
            printf("%u\n", (size_t)btree_leaf_value(&btree_geo32, node, i-1));
        }
    }

    for (size_t idx : indices) {
        btree_insert(&tree, &btree_geo32, &idx, (void*)data[idx]);

        //total += diff_us;
        //n++;
        //if (diff_us > max)
        //    max = diff_us;
    }

    printf("First: %u\n", (size_t)btree_first(&tree, &btree_geo32, &key, &node));
    printf("Prev: %p\n", btree_leaf_prev(&btree_geo32, node));

    for ( ; node; node = btree_leaf_prev(&btree_geo32, node)) {
        int fill = getfill(&btree_geo32, node, 0);
        for (int i = fill; i > 0; i--) {
            printf("%u\n", (size_t)btree_leaf_value(&btree_geo32, node, i-1));
        }
    }

    //clock_gettime(CLOCK_MONOTONIC_RAW, &after);

    //u32 diff_us = (u32)(after.tv_sec - before.tv_sec) * 1000000 + (u32)(after.tv_nsec - before.tv_nsec) / 1000;

    //printf("Diff: %u us\n", diff_us);

    //printf("Height: %d\n", tree.height);

    //size_t count = btree_visit(&tree, &btree_geo32, 0, [](void* elem, size_t udata1, size_t* key, size_t index) {
        //printf("Elem: %u, Udata1: %u, Key: %p, Index: %u\n", (size_t)elem, udata1, key, index);
    //});

    //printf("Count: %u, Height: %d\n", count, tree.height);

    return 0;
}

