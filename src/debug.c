#include <stdio.h>
#include <debug.h>

void debug(char* s) {

#ifdef DEBUG
    fprintf(stderr, "[DEBUG] %s\n", s);
#endif

}
