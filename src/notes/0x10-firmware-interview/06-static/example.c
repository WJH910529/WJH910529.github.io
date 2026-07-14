#include <stdio.h>

/* File scope + static: this name has internal linkage. */
static int module_total;

/* A static function can only be named from this translation unit. */
static void add_to_total(int value)
{
    /* Block scope + static: initialized once and retained between calls. */
    static unsigned int call_count;

    ++call_count;
    module_total += value;

    printf("call %u: add %d, module_total = %d\n",
           call_count,
           value,
           module_total);
}

int main(void)
{
    printf("module_total before: %d\n", module_total);

    add_to_total(5);
    add_to_total(-2);
    add_to_total(10);

    printf("module_total after : %d\n", module_total);
    return 0;
}
