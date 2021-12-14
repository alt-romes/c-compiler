#ifndef _AST_H
#define _AST_H

#include "environment.h"

/* Declaration List */

typedef enum type_qualifier {
    CONST = 21
} type_qualifier_t;

typedef enum type_specifier {
    CHAR = 0b1,
    SHORT = 0b10,
    INT = 0b100,
    LONG = 0b1000,
    SIGNED = 0,
    UNSIGNED = 0b1000000
} type_specifier_t;

int is_int_type_unsigned(enum type_specifier);

#define EMPTY_DEC_SPECS { .tq = -1, .ts = -1 }
struct declaration_specifiers {
    type_qualifier_t tq;
    type_specifier_t ts;
};

struct declaration {
    char* id;
    struct declaration_specifiers ds;
    struct node* node;
};

typedef struct declaration_list {
    struct declaration* declarations;
    int size;
} declaration_list_t; 

declaration_list_t* create_declaration_list();
declaration_list_t* declaration_list_assoc(declaration_list_t* e, struct declaration d);
declaration_list_t* declaration_list_merge(declaration_list_t* src, declaration_list_t* dst); // merge two environments by copying all associations from src to dst, freeing src, and keeping dst's parent
declaration_list_t* add_declaration_specifiers(declaration_list_t* decs, struct declaration_specifiers ds);

/*
 * All `*_node` structures must start with a node_type_t field so that they can
 * be cast from and to `struct node`
 */

typedef enum node_type {
    NUM,
    ID,
    ADD,
    SUB,
    MUL,
    DIV,
    UMINUS,
    BLOCK,
    FUNCTION
} node_type_t;

typedef struct node {
    node_type_t type;
    type_specifier_t ts;
} node_t;

typedef struct num_node {
    node_type_t type;
    type_specifier_t ts;
    int value;
} num_node_t;

typedef struct id_node {
    node_type_t type;
    type_specifier_t ts;
    char* value;
} id_node_t;

typedef struct unary_node {
    node_type_t type;
    type_specifier_t ts;
    struct node* child;
} unary_node_t;

typedef struct binary_node {
    node_type_t type;
    type_specifier_t ts;
    struct node* left;
    struct node* right;
} binary_node_t;

typedef struct block_node {
    node_type_t type;
    type_specifier_t ts;
    declaration_list_t* declaration_list;
    struct node* body;
} block_node_t;

typedef struct function_node {
    node_type_t type;
    type_specifier_t ts;
    char* name;
    struct node* body;
} function_node_t;

node_t* create_node_literal(node_type_t, type_specifier_t, void* literal_value);
node_t* create_node1(node_type_t, node_t*);
node_t* create_node2(node_type_t, node_t*, node_t*);
node_t* create_node_block(node_type_t, declaration_list_t*, node_t*);
node_t* create_node_function(node_type_t, type_specifier_t, char*, node_t*);

void free_ast(node_t* root);

#endif
