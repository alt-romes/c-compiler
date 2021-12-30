int main() {

    int y = 1;
    int* z = &y;
    int * const * x = &z;
    **x = 4;
    *x = &z;
    return **x;
}
