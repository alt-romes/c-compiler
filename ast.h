#ifndef _AST_H
#define _AST_H

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
    union {
        int num_value;
        char* id_value;
        struct node* child;
        struct {
            struct node* left;
            struct node* right;
        } children;
        struct {
            char* id;
            struct node* left;
            struct node* right;
        } def;
    };
} node_t;

node_t* create_node_literal(node_type_t type, void* literal_value);
node_t* create_node1(node_type_t type, node_t* n);
node_t* create_node2(node_type_t type, node_t* l, node_t* r);
node_t* create_node_def(node_type_t type, char* id, node_t* l, node_t* r);

void free_ast(node_t* root);

#endif
