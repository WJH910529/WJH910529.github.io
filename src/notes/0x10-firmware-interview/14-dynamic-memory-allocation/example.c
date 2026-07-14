#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    void *ptr = malloc(0);

    if (ptr == NULL) {
        puts("malloc(0): NULL");
    } else {
        puts("malloc(0): non-NULL");
    }

    puts("calling free(ptr)");
    free(ptr);
    puts("free(ptr): completed");

    return 0;
}
