#include <stdio.h>

#ifndef DEMO_LIMIT
#define DEMO_LIMIT 3U
#endif

int main(void)
{
    unsigned int iteration = 0U;

    puts("enter main loop");
    for (;;) {
        ++iteration;
        printf("iteration %u: poll devices and handle events\n", iteration);

#if DEMO_LIMIT > 0U
        if (iteration >= DEMO_LIMIT) {
            puts("demo only: break so the host test can finish");
            break;
        }
#endif
    }

    printf("loop body ran %u times\n", iteration);
    return 0;
}
