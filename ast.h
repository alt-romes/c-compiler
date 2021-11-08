#ifndef _AST_H
#define _AST_H

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
    DEF
} node_type_t ;

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

typedef struct def_node {
    node_type_t type;
    char* id;
    struct node* left;
    struct node* right;
} def_node_t;

node_t* create_node_literal(node_type_t type, void* literal_value);
node_t* create_node1(node_type_t type, node_t* n);
node_t* create_node2(node_type_t type, node_t* l, node_t* r);
node_t* create_node_def(node_type_t type, char* id, node_t* l, node_t* r);

void free_ast(node_t* root);

#endif
