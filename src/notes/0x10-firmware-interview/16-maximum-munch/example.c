#include <stdio.h>

int main(void)
{
    int a = 5;
    int b = 7;
    int c;
    int x = 5;
    int y = 7;
    int d;

    c = a+++b;
    d = x++ + y;

    printf("after c = a+++b: a=%d, b=%d, c=%d\n", a, b, c);
    printf("after d = x++ + y: x=%d, y=%d, d=%d\n", x, y, d);
    puts(a == x && b == y && c == d ? "same result? yes"
                                    : "same result? no");

    return 0;
}
