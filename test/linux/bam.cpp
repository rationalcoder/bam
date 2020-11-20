#include <bam.h>
#include <stdio.h>

void
test_big_arrays()
{
    bam::big_array<int> ints;
    for (int i = 0; i < 10000; i++) {
        ints.add(i);
    }

    int count = 0;
    for (int i : ints) {
        bam_assert(i == count++);
    }

    printf("Capacity: %u\n", ints._capacityInBytes);
    printf("[%d] = %d\n", 9000, ints[9000]);
    ints.clear();
    printf("Capacity: %u, Size: %u\n", ints._capacityInBytes, ints.size());

    for (int i = 0; i < 20000; i++) {
        ints.add(i);
    }
    printf("Capacity: %u, Size: %u\n", ints._capacityInBytes, ints.size());
    printf("[%d] = %d\n", 15000, ints[15000]);
}

int main(void)
{
    bam::init_context();

    bam::string s = "lkjsdf";
    printf("String: %s\n", s.c_str());

    bam::allocate_arena("Test", 4_KB);

    bam::string_builder builder(bam::temp_memory());
    builder.append_fmt("Stuff to append: %d %f", 10, 15.2f);

    printf("Builder content: %s\n", builder.str().c_str());

    bam::blist<int> ints(bam::temp_memory());
    for (int i = 0; i < 33; i++) {
        ints.add(i);
    }

    for (int i : ints) {
        printf("Int: %d\n", i);
    }

    test_big_arrays();

    return 0;
}

#define BAM_IMPLEMENTATION
#include <bam.h>
