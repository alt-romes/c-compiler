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

    /* Function type has the return type in the lower 8 bits,
     * and the type parameters are in an associated enum type list */
    FUNCTION_TYPE = 0x200,

    /* A type can be reference of another type, but can also be reference of
     * another reference of another type.  To encode this all in just one
     * number, each reference type adds to the enum type, so 1 reference will
     * be 0x1000 | the type it references, 2 references will be 0x1000 + 0x1000
     * | the type it references.  Essentially, the bits between 0x8000 and
     * 0x1000 represent the length of the reference chain, which allows for a
     * maximum of 16 chained references. This should be enough for now, but an
     * expansion for up to 256 is currently possible.
     */
    REFERENCE = 0x1000, // until 0x8000
    IS_REFERENCE = 0xF000, // type & IS_REFERENCE to get if value is a reference

    /* Qualifiers */
    CONST = 0x80000000
}; 

int is_int_type_unsigned(enum type t);
int type_compare(enum type l, enum type r);
enum type ref_of(enum type);
enum type deref(enum type);
int reference_chain_length(enum type t);

#endif
