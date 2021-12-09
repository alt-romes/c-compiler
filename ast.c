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

node_t* create_node_block(node_type_t type, declaration_list_t* declaration_list, node_t* b) {

    block_node_t* node = (block_node_t*)new_node(type);
    node->declaration_list = declaration_list;
    node->body = b;
    return (node_t*)node;
}

void free_ast(node_t* node) {


    switch (node->type) {
        case BLOCK: {
                declaration_list_t* dae = ((block_node_t*)node)->declaration_list;
                for (int i = 0; i < dae->size; i++) {
                    free(dae->declarations[i].id);
                    if (dae->declarations[i].node != NULL)
                        free(dae->declarations[i].node);
                }

                free(dae->declarations);
                free(dae);
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


/* Declaration List */
declaration_list_t* create_declaration_list() { return malloc(sizeof(declaration_list_t));}

declaration_list_t* declaration_list_assoc(declaration_list_t* e, struct declaration d) {
    if (!e->size % DEFAULT_ENVIRONMENT_SIZE)
        e->declarations = realloc(e->declarations, (e->size+DEFAULT_ENVIRONMENT_SIZE)*sizeof(struct declaration));

    e->declarations[e->size++] = d;

    return e;
}

declaration_list_t* declaration_list_merge(declaration_list_t* src, declaration_list_t* dst) {

    for (int i = 0; i < src->size; i++)
        declaration_list_assoc(dst, src->declarations[i]);

    free(src->declarations);
    free(src);
    
    return dst;
}

declaration_list_t* add_declaration_specifiers(declaration_list_t* decs, struct declaration_specifiers ds) {
    
    for (struct declaration* d = decs->declarations; d < d+decs->size; d++) d->ds = ds;

    return decs;
}
