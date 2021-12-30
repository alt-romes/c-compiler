#include <stdio.h>

char fff(int x, int* y) {

    return x + *y;
}

int main() {

    int x = 1;
    char (*fun)(int, int*) = (char (*)(int, int*))fff;

    printf("%d\n", fun(x, &x));
    return 0;
}
