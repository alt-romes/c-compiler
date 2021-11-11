#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "environment.h"
#include "ast.h"

node_t* new_node(node_type_t type) {

    node_t* node;

    switch (type) {
        case BLOCK:
            node = malloc(sizeof(block_node_t));
            break;
        case ADD:
        case SUB:
        case MUL:
        case DIV:
            node = malloc(sizeof(binary_node_t));
            break;
        case UMINUS:
            node = malloc(sizeof(unary_node_t));
            break;
        case ID:
            node = malloc(sizeof(id_node_t));
            break;
        case NUM:
            node = malloc(sizeof(num_node_t));
            break;
    }

    node->type = type;

    return node;
}

node_t* create_node_literal(node_type_t type, void* literal_value) {
    
    node_t* node = new_node(type);
    
    switch (type) {
        case NUM:
            ((num_node_t*)node)->value = (int)(intptr_t)literal_value;
            break;
        case ID:
            ((id_node_t*)node)->value = (char*)literal_value;
            break;
        default:
            fprintf(stderr, "ERROR: Literal node should have type NUM or ID!\n");
            exit(1);
    }

    return node;
}

node_t* create_node1(node_type_t type, node_t* n) {

    unary_node_t* node = (unary_node_t*)new_node(type);
    node->child = n;
    return (node_t*)node;
}

node_t* create_node2(node_type_t type, node_t* l, node_t* r) {
     
    binary_node_t* node = (binary_node_t*)new_node(type);
    node->left = l;
    node->right = r;
    return (node_t*)node;
}

node_t* create_node_block(node_type_t type, environment_t* declarations_ast_env, node_t* b) {

    block_node_t* node = (block_node_t*)new_node(type);
    node->declarations_ast_env = declarations_ast_env;
    node->body = b;
    return (node_t*)node;
}


void free_ast(node_t* node) {


    switch (node->type) {
        case BLOCK: {
                free_environment(((block_node_t*)node)->declarations_ast_env);
                free(((block_node_t*)node)->body);
                break;
            }
        case ADD:
        case SUB:
        case MUL:
        case DIV:
            free_ast(((binary_node_t*)node)->left);
            free_ast(((binary_node_t*)node)->right);
            break;
        case UMINUS:
            free_ast(((unary_node_t*)node)->child);
            break;
        case ID:
            free(((id_node_t*)node)->value);
            break;
        case NUM:
            break;
    }

    free(node);
}
