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
            node->num_value = (int)(intptr_t)literal_value;
            break;
        case ID:
            node->id_value = (char*)literal_value;
            printf("assigned to node id_value: %s\n", node->id_value);
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

node_t* create_node_def(node_type_t type, char* id, const node_t* l, const node_t* r) {

    printf("create node def with id (%s)!\n", id);
    
    node_t* node = new_node(type);
    node->def.id = id;
    node->def.left = l;
    node->def.right = r;
    return node;
}


void free_ast(node_t* node) {

    switch (node->type) {
        case DEF:
            free(node->def.id);
            free((node_t*)node->def.left);
            free((node_t*)node->def.right);
            break;
        case ADD:
        case SUB:
        case MUL:
        case DIV:
            // TODO: to const or not to const? should the data structure have const children?
            free_ast((node_t*)node->children.left);
            free_ast((node_t*)node->children.right);
            break;
        case UMINUS:
            free_ast((node_t*)node->child);
            break;
        case ID:
            free(node->id_value);
            break;
        case NUM:
            break;
    }

    free((void*)node);

}
