#include <stddef.h>
#include <stdio.h>

static int sum_samples(const int *samples, size_t count)
{
    size_t index;
    int total = 0;

    for (index = 0; index < count; ++index) {
        total += samples[index];
    }

    return total;
}

int main(void)
{
    int first = 10;
    int second = 20;
    const int limit = 100;
    const int *pointer_to_const = &first;
    int * const const_pointer = &first;
    const int * const const_pointer_to_const = &second;
    int samples[] = { 3, 5, 7 };

    printf("limit: %d\n", limit);

    printf("pointer_to_const -> %d\n", *pointer_to_const);
    pointer_to_const = &second;
    printf("after pointer move -> %d\n", *pointer_to_const);

    *const_pointer = 11;
    printf("value changed through const pointer: %d\n", first);

    printf("const pointer to const data -> %d\n",
           *const_pointer_to_const);
    printf("sum of read-only samples: %d\n",
           sum_samples(samples, sizeof samples / sizeof samples[0]));

    return 0;
}
