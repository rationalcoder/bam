#include <bam.h>
#include <stdio.h>

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

    for (int i: ints) {
        printf("Int: %d\n", i);
    }

    return 0;
}

#define BAM_IMPLEMENTATION
#include <bam.h>
