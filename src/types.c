#include <stdint.h>
#include <stdio.h>
#include "ast.h"
#include "types.h"

int is_type_unsigned(type_t t) {

    return t.t & UNSIGNED;
}

/* Returns 0 if the same, negative if l is smaller, positive if l is larger */
int type_compare(type_t l, type_t r) {

    // TODO: is the result of binary operation on numbers signed or unsigned?
    // TODO: compare pointer types?...
    return (l.t & 0xff) - (r.t & 0xff);
}

type_t type_from(enum type t) {
    return (type_t){ t };
}

type_t ref_of(type_t t) {

    /* return t | REFERENCE; */
}

type_t deref(type_t t) {
    
    /* return t ^ REFERENCE; */
}
