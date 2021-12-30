#ifndef _TYPES_H
#define _TYPES_H

/* Types */

enum type {

    /* enum type is a qualifier-specifier type indicator used for typechecking
     * and code generation.  all constants correspond to a bit, meaning we can
     * have multiple qualifiers-specifiers encoded in the same enum by doing
     * the bit OR operation between said different qualifiers-specifiers */

    /* Specifiers */
    VOID = 0, /* Is this OK being zero? :) */
    I1 = 0b1,
    CHAR = 0b10,
    SHORT = 0b100,
    INT = 0b1000,
    LONG = 0b10000,
    SIGNED = 0,
    UNSIGNED = 0x100,

    /* A type can be reference of another type, but can also be reference of
     * another reference of another type.  To encode this all in just one
     * number, each reference type adds to the enum type, so 1 reference will
     * be 0x1000 | the type it references, 2 references will be 0x1000 + 0x1000
     * | the type it references.  Essentially, the bits between 0x8000 and
     * 0x1000 represent the length of the reference chain, which allows for a
     * maximum of 16 chained references. This should be enough for now, but an
     * expansion for up to 256 is currently possible.
     */
    POINTER = 0x1000, // until 0x8000

    /* Function type has the return type in the lower 9 bits,
     * and the type parameters are in an associated enum type list */
    FUNCTION_TYPE = 0x2000,

    /* Qualifiers */
    CONST = 0x40000000,

    /* Undefined :P */
    UNDEFINED = -1
}; 

typedef struct type_s {
    enum type t;
} type_t;

struct function_type {
    enum type t;
    type_t ret;
    struct args_list* args;
};

struct pointer_type {
    enum type t;
    type_t pointed;
};

int is_type_unsigned(type_t t);
int type_compare(type_t l, type_t r);

type_t type_from(enum type t);

type_t ref_of(type_t);
type_t deref(type_t);

struct type_s create_type_pointer(enum type pointer_type_and_qualifiers);
struct type_s create_type_function(enum type function_type_and_qualifiers, struct args_list* args);

/* (Recursively) Set the base type of an "in-construction" type.
 * Setting the base type of a...
 *  POINTER -> is setting the pointed to type
 *  FUNCTION -> is setting the return type
 *  UNDEFINED -> is setting the type directly
 */
struct type_s set_base_type(struct type_s, struct type_s);

#endif
