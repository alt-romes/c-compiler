#include <stdio.h>
#include <debug.h>

void debug(char* s) {

#ifdef DEBUG
    fprintf(stderr, "[DEBUG] %s\n", s);
#endif

}

void debug_type(char* s, type_t t) {

#ifdef DEBUG
    switch (t->t & (POINTER | FUNCTION_TYPE)) {
        case POINTER:
            fprintf(stderr, "[DEBUG TYPE] %s | Pointer %x\n", s, t->t);
            debug_type("pointing to...", ((pointer_type_t)t)->pointed);
            break;
        case FUNCTION_TYPE:
            fprintf(stderr, "[DEBUG TYPE] %s | Function %x\n", s, t->t);
            debug_type("returning...", ((function_type_t)t)->ret);
            break;
        default:
            fprintf(stderr, "[DEBUG TYPE] %s | %x\n", s, t->t);
            break;
    }
#endif

}

void debugf1(char* f, char* s) {

#ifdef DEBUG
    fprintf(stderr, "[DEBUG] ");
    fprintf(stderr, f, s);
    fprintf(stderr, "\n");
#endif

}
