#include <stdio.h>

static int twice(int value)
{
    return value * 2;
}

static int triple(int value)
{
    return value * 3;
}

static int square(int value)
{
    return value * value;
}

int main(void)
{
    {
        int a = 10;
        printf("(a) integer: %d\n", a);
    }

    {
        int value = 20;
        int *a = &value;
        printf("(b) pointer to integer: %d\n", *a);
    }

    {
        int value = 30;
        int *pointer = &value;
        int **a = &pointer;
        printf("(c) pointer to pointer: %d\n", **a);
    }

    {
        int a[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        printf("(d) array of integers: a[0]=%d, a[9]=%d\n",
               a[0], a[9]);
    }

    {
        int first = 40;
        int second = 50;
        int third = 60;
        int *a[10] = { &first, &second, &third };

        ++*a[0];
        ++*a[1];
        ++*a[2];
        printf("(e) array of pointers: %d, %d, %d\n",
               *a[0], *a[1], *a[2]);
    }

    {
        int rows[2][10] = {
            { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 },
            { 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 }
        };
        int (*a)[10] = rows;

        printf("(*a)[0] is %d\n", (*a)[0]);
        printf("*(*a + 1) is %d\n", *(*a + 1));
        printf("*(*(a + 1)) is %d\n", *(*(a + 1)));
        printf("(*(a + 1))[1] is %d\n", (*(a + 1))[1]);
        printf("(f) pointer to array: first row %d..%d\n",
               (*a)[0], (*a)[9]);
        ++a;
        printf("    after ++a: second row %d..%d\n",
               (*a)[0], (*a)[9]);

    }

    {
        int (*a)(int) = twice;
        printf("(g) function pointer: twice(7)=%d\n", a(7));
    }

    {
        int (*a[10])(int) = { twice, triple, square };

        printf("(h) function pointer array: %d, %d, %d\n",
               a[0](4), a[1](4), a[2](4));
    }

    return 0;
}
