#include <stdio.h>

struct s {
    int value;
};

#define dPS struct s *
typedef struct s *tPS;

#define TYPE_NAME(value) _Generic((value), \
    struct s *: "pointer to struct s",       \
    struct s: "struct s")

int main(void)
{
    struct s first = {10};
    struct s third = {30};
    struct s fourth = {40};
    dPS p1, p2;
    tPS p3, p4;

    p1 = &first;
    p2.value = 20;
    p3 = &third;
    p4 = &fourth;

    printf("p1: %s, value=%d\n", TYPE_NAME(p1), p1->value);
    printf("p2: %s, value=%d\n", TYPE_NAME(p2), p2.value);
    printf("p3: %s, value=%d\n", TYPE_NAME(p3), p3->value);
    printf("p4: %s, value=%d\n", TYPE_NAME(p4), p4->value);

    return 0;
}
