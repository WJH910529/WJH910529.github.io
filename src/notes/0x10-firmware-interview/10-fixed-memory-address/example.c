#include <stdint.h>
#include <stdio.h>

struct simulated_peripheral {
    volatile uint16_t data;
    volatile uint16_t neighbor;
};

int main(void)
{
    struct simulated_peripheral device = {
        UINT16_C(0x0000),
        UINT16_C(0x1234)
    };
    volatile uint16_t * const register_pointer = &device.data;

    printf("simulated register width: %lu bits\n",
           (unsigned long)(sizeof *register_pointer * 8U));
    printf("before write: 0x%04lX\n",
           (unsigned long)*register_pointer);

    *register_pointer = UINT16_C(0xAA55);

    printf("after write : 0x%04lX\n",
           (unsigned long)*register_pointer);
    printf("neighbor    : 0x%04lX\n",
           (unsigned long)device.neighbor);

    return 0;
}
