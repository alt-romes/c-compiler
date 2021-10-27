#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

node_t* new_node(node_type_t type) {
    node_t* node = malloc(sizeof(node_t));
    node->type = type;
    return node;
}

node_t* create_node_literal(node_type_t type, const void* literal_value) {
    
    node_t* node = new_node(type);
    
    switch (type) {
        case NUM:
            node->num_value = *(int*)literal_value;
            break;
        case ID:
            node->id_value = *(char**)literal_value;
            break;
        default:
            fprintf(stderr, "ERROR: Literal node should have type NUM or ID!\n");
            exit(1);
    }
    return node;
}

node_t* create_node1(node_type_t type, const node_t* n) {

    node_t* node = new_node(type);
    node->child = n;
    return node;
}

node_t* create_node2(node_type_t type, const node_t* l, const node_t* r) {
     
    node_t* node = new_node(type);
    node->children.left = l;
    node->children.right = r;
    return node;
}


void free_ast(node_t* root) {

    printf("TODO: Free memory!!!\n");
}
