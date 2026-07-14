#include <limits.h>
#include <stdio.h>

int main(void)
{
    unsigned int a = 6;
    int b = -20;
    unsigned int converted_b = (unsigned int)b;
    unsigned int sum = a + b;

    printf("unsigned int object bits = %u\n",
           (unsigned int)(sizeof(unsigned int) * CHAR_BIT));
    printf("UINT_MAX = %u\n", UINT_MAX);
    printf("b as unsigned int = %u\n", converted_b);
    printf("a + b as unsigned int = %u\n", sum);
    (a + b > 6) ? puts("comparison result: > 6")
                : puts("comparison result: <= 6");

    return 0;
}
