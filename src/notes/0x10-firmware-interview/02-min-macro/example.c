#include <stdio.h>

#define MIN(A, B) ((A) <= (B) ? (A) : (B))

static inline int min_int(int a, int b)
{
    return a <= b ? a : b;
}

int main(void)
{
    int macro_value = 3;
    int inline_value = 3;
    int limit = 10;
    int macro_result;
    int inline_result;

    printf("MIN(8, 3) = %d\n", MIN(8, 3));

    macro_result = MIN(macro_value++, limit);
    inline_result = min_int(inline_value++, limit);

    printf("macro: result=%d, value_after=%d\n",
           macro_result, macro_value);
    printf("inline: result=%d, value_after=%d\n",
           inline_result, inline_value);

    return 0;
}
