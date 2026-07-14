#include <stdint.h>
#include <stdio.h>

#define BIT(n) (UINT32_C(1) << (n))
#define BIT3 BIT(3)

static void print_value(const char *label, uint32_t value)
{
    unsigned int bit;

    printf("%-13s 0x%02lX  ", label, (unsigned long)value);
    for (bit = 8; bit > 0; --bit) {
        putchar((value & BIT(bit - 1)) != 0U ? '1' : '0');
    }
    putchar('\n');
}

int main(void)
{
    const uint32_t original = UINT32_C(0xA5);
    uint32_t value = original;
    int other_bits_unchanged;

    print_value("initial:", value);

    value |= BIT3;
    print_value("set bit 3:", value);

    other_bits_unchanged =
        (value & ~BIT3) == (original & ~BIT3);

    value &= ~BIT3;
    print_value("clear bit 3:", value);

    other_bits_unchanged = other_bits_unchanged &&
        ((value & ~BIT3) == (original & ~BIT3));
    printf("other bits unchanged: %s\n",
           other_bits_unchanged ? "yes" : "no");

    return 0;
}
