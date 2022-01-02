int main() {

    int volatile y = 2;

    int * const * volatile * x;

    y = 3;

    return ***x;
}
