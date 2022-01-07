#ifndef _TYPES_H
#define _TYPES_H

/* Types */

enum type {

    /* enum type is a qualifier-specifier type indicator used for typechecking
     * and code generation.  all constants correspond to a bit, meaning we can
     * have multiple qualifiers-specifiers encoded in the same enum by doing
     * the bit OR operation between said different qualifiers-specifiers */

    /* Specifiers */
    VOID = 0x0,
    I1 = 0x1,
    CHAR = 0x2,
    SHORT = 0x4,
    INT = 0x8,
    LONG = 0x10,

    POINTER = 0x1000,

    FUNCTION_TYPE = 0x2000,
    ARRAY_TYPE = 0x4000,

    SIGNED = 0,
    UNSIGNED = 0x10000,

    /* Qualifiers */
    VOLATILE = 0x10000000,
    CONST = 0x20000000,

    /* Global */
    GLOBAL = 0x40000000,

    /* Undefined :P */
    UNDEFINED = 0x80000000
}; 

/*
 * Type structures are heap allocated.
 * A global structure is kept with all allocated types.
 * Simple types are to be reused,
 * while complex types such as POINTER and FUNCTION_TYPE,
 * should be allocated new everytime.
 * To free the types, call `free_all_types` that goes over the
 * global structure and frees all allocated types.
 */

typedef struct type_s {
    enum type t;
} * type_t;

typedef struct function_type {
    enum type t;
    type_t ret;
    struct args_list* args;
} * function_type_t;

typedef struct pointer_type {
    enum type t;
    type_t pointed;
} * pointer_type_t;

typedef struct array_type {
    enum type t;
    type_t elems;
    struct node* size; 
} * array_type_t;

int is_type_unsigned(type_t t);
int is_type_const(type_t t);
int is_type_pointer(type_t t);
int type_compare(type_t l, type_t r);

type_t type_from(enum type t);

type_t ref_of(type_t);
type_t deref(type_t);

type_t create_type_pointer(enum type pointer_type_and_qualifiers);
type_t create_type_function(enum type function_type_and_qualifiers, struct args_list* args);
type_t create_type_array(enum type array_type_and_qualifiers, struct node* size);

/* (Recursively) Set the base type of an "in-construction" type.
 * Setting the base type of a...
 *  POINTER -> is setting the pointed to type
 *  FUNCTION -> is setting the return type
 *  UNDEFINED -> is setting the type directly
 */
type_t set_base_type(type_t, type_t);
type_t extend_base_type(type_t, enum type);
enum type get_base_type(type_t);

void free_all_types();

#endif
