#ifndef _AST_H
#define _AST_H

#include "environment.h"

/* Declaration List */

typedef enum type_qualifier {
    CONST = 21
} type_qualifier_t;

typedef enum type_specifier {
    INT = 22
} type_specifier_t;

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
    BLOCK
} node_type_t;

typedef struct node {
    node_type_t type;
} node_t;

typedef struct num_node {
    node_type_t type;
    int value;
} num_node_t;

typedef struct id_node {
    node_type_t type;
    char* value;
} id_node_t;

typedef struct unary_node {
    node_type_t type;
    struct node* child;
} unary_node_t;

typedef struct binary_node {
    node_type_t type;
    struct node* left;
    struct node* right;
} binary_node_t;

typedef struct block_node {
    node_type_t type;
    declaration_list_t* declaration_list;
    struct node* body;
} block_node_t;

node_t* create_node_literal(node_type_t, void* literal_value);
node_t* create_node1(node_type_t, node_t*);
node_t* create_node2(node_type_t, node_t*, node_t*);
node_t* create_node_block(node_type_t, declaration_list_t*, node_t*);

void free_ast(node_t* root);

#endif
