#include <stdio.h>

#if !defined(TARGET_DEMO_BOARD)
#error "TARGET_DEMO_BOARD must be defined"
#endif

int main(void)
{
    puts("target: demo board");
    return 0;
}
