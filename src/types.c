#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "types.h"

int is_type_unsigned(type_t t) {

    /* return get_base_type(t) & UNSIGNED; */
    return t->t & UNSIGNED;
}

int is_type_const(type_t t) {
    // TODO: Get base type?
    return t->t & CONST;
}

int is_type_pointer(type_t t) {
    // TODO: Same as 2 above
    return t->t & POINTER;
}

/* Returns 0 if the same, negative if l is smaller, positive if l is larger */
int type_compare(type_t l, type_t r) {

    // TODO: is the result of binary operation on numbers signed or unsigned?
    // TODO ... wrong?
    return (l->t & 0xff) - (r->t & 0xff);
}

type_t type_from(enum type t) {
    type_t x = malloc(sizeof(struct type_s));
    x->t = t;
    return x;
}

type_t ref_of(type_t t) {

    return set_base_type(create_type_pointer(POINTER), t);
}

type_t deref(type_t t) {
    
    // TODO: where free?
    return ((pointer_type_t)t)->pointed;
}

type_t create_type_pointer(enum type pointer_type_and_qualifiers) {
    
    pointer_type_t p = malloc(sizeof(struct pointer_type));
    p->t = pointer_type_and_qualifiers;
    p->pointed = type_from(UNDEFINED);
    return (type_t)p;
}

type_t create_type_function(enum type function_type_and_qualifiers, struct args_list* args) {

    function_type_t f = malloc(sizeof(struct function_type));
    f->t = function_type_and_qualifiers;
    f->args = args;
    f->ret = type_from(UNDEFINED);
    return (type_t)f;
}

/* (Recursively) Set the base type of an "in-construction" type.
 * Setting the base type of a...
 *  POINTER -> is setting the pointed to type
 *  FUNCTION_TYPE -> is setting the return type
 *  UNDEFINED -> is setting the type directly
 */
type_t set_base_type(type_t t, type_t b) {

    switch (t->t & (POINTER | FUNCTION_TYPE | UNDEFINED)) {
        case UNDEFINED:
            free(t);
            return b;
        case POINTER:
            ((pointer_type_t)t)->pointed = set_base_type(((pointer_type_t)t)->pointed, b);
            return t;
        case FUNCTION_TYPE:
            ((function_type_t)t)->ret = set_base_type(((function_type_t)t)->ret, b);
            return t;
        default:
            fprintf(stderr, "Cannot set base type of type %x to %x!\n", t->t, b->t);
            exit(31);
    }
}

type_t extend_base_type(type_t t, enum type e) {

    switch (t->t & (POINTER | FUNCTION_TYPE)) {
        case POINTER:
            ((pointer_type_t)t)->pointed = extend_base_type(((pointer_type_t)t)->pointed, e);
            return t;
        case FUNCTION_TYPE:
            ((function_type_t)t)->ret = extend_base_type(((function_type_t)t)->ret, e);
            return t;
        default:
            t->t |= e;
            return t;
    }

}


/* void free_type(type_t t) { */

/*     switch (t->t & (POINTER | FUNCTION_TYPE)) { */
/*         case POINTER: */
/*             free_type(((pointer_type_t)t)->pointed); */
/*             break; */
/*         case FUNCTION_TYPE: */
/*             free_type(((function_type_t)t)->ret); */

/*             // Free args list */
/*             for (int i = ((function_type_t)t)->args->size; i --> 0;) { */
/*                 free_type(((function_type_t)t)->args->args[i].ts); */
/*                 free(((function_type_t)t)->args->args[i].id); */
/*             } */

/*             if (((function_type_t)t)->args->args != NULL) */
/*                 free(((function_type_t)t)->args->args); */

/*             free(((function_type_t)t)->args); */

/*             break; */
/*         default: */
/*             break; */
/*     } */

/*     free(t); */
/* } */
