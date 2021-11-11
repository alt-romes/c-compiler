#ifndef _AST_H
#define _AST_H

#include "environment.h"

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
    environment_t* declarations_ast_env;
    struct node* body;
} block_node_t;

node_t* create_node_literal(node_type_t, void* literal_value);
node_t* create_node1(node_type_t, node_t*);
node_t* create_node2(node_type_t, node_t*, node_t*);
node_t* create_node_block(node_type_t, environment_t*, node_t*);

void free_ast(node_t* root);

#endif
