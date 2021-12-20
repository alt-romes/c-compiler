#ifndef _TYPES_H
#define _TYPES_H

/* Types */

enum type {

    /* enum type is a qualifier-specifier type indicator used for typechecking
     * and code generation.  all constants correspond to a bit, meaning we can
     * have multiple qualifiers-specifiers encoded in the same enum by doing
     * the bit OR operation between said different qualifiers-specifiers */

    /* Specifiers */
    I1 = 0b1,
    CHAR = 0b10,
    SHORT = 0b100,
    INT = 0b1000,
    LONG = 0b10000,
    SIGNED = 0,
    UNSIGNED = 0x100,

    /* Qualifiers */
    CONST = 0x8000
}; 
#define EMPTY_DEC_SPECS -1

int is_int_type_unsigned(enum type t);
int type_compare(enum type l, enum type r);

#endif
