#include <limits.h>
#include <stdio.h>

#define SECONDS_PER_YEAR (60UL * 60UL * 24UL * 365UL)

_Static_assert(SECONDS_PER_YEAR == 31536000UL,
               "SECONDS_PER_YEAR has an unexpected value");

int main(void)
{
    printf("SECONDS_PER_YEAR = %lu\n", SECONDS_PER_YEAR);
    printf("sizeof(int) = %lu bytes\n", (unsigned long)sizeof(int));
    printf("sizeof(unsigned long) = %lu bytes\n",
           (unsigned long)sizeof(unsigned long));
    printf("ULONG_MAX = %lu\n", ULONG_MAX);

    return 0;
}
