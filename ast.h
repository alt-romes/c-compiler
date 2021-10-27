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
        const struct node* child;
        struct {
            const struct node* left;
            const struct node* right;
        } children;
        struct {
            const char* id;
            const struct node* left;
            const struct node* right;
        } def;
    };
} node_t;

node_t* create_node_literal(node_type_t type, const void* literal_value);
node_t* create_node1(node_type_t type, const node_t* n);
node_t* create_node2(node_type_t type, const node_t* l, const node_t* r);
node_t* create_node_def(node_type_t type, const char* id, const node_t* l, const node_t* r);

void free_ast(node_t* root);

#endif
