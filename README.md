## Another C to LLVM Compiler

This is a C to LLVM compiler written in C.

The supported features can be seen by examining the parser (yacc) file, because the unsupported features are still commented in the parser, while all other "parseable" features are complete.

A broad range of the features have associated tests under `/compiler-tests`

It already supports compilation of this <interesting> program:
```c
void* (*f(void* (*(*p)(int (*)[2]))[]))[] {

    int y[2];
    void* (*x)[] = (void* (*)[])p(&y);
    return x;
}
```
