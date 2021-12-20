#include <stdint.h>
#include <stdio.h>
#include "ast.h"
#include "types.h"

int is_int_type_unsigned(enum type t) {

    return t & UNSIGNED;
}

/* Returns 0 if the same, negative if l is smaller, positive if l is larger */
int type_compare(enum type l, enum type r) {

    // TODO: is the result of binary operation on numbers signed or unsigned?
    return (l & 0xff) - (r & 0xff);
}

