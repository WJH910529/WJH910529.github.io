#include <limits.h>
#include <stdio.h>

int main(void)
{
    unsigned int zero = 0u;
    unsigned int hard_coded = 0xFFFFu;
    unsigned int compzero = ~zero;

    printf("unsigned int object bits = %u\n",
           (unsigned int)(sizeof(unsigned int) * CHAR_BIT));
    printf("UINT_MAX = %u\n", UINT_MAX);
    printf("0xFFFF = %u (0x%08X)\n", hard_coded, hard_coded);
    printf("~zero = %u (0x%08X)\n", compzero, compzero);
    puts(hard_coded == compzero ? "same value? yes" : "same value? no");

    return 0;
}
