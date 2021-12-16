#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "ast.h"
#include "environment.h"
#include "parse_utils.h"

int eval(const node_t* node, environment_t* e) {
    switch (node->type) {

        case FUNCTION:
            return eval(((function_node_t*)node)->body, e);

        case ID:
            return find(e, ((id_node_t*)node)->value).integer;

        case BLOCK: {
            environment_t* scope_env = beginScope(e);
            
            block_node_t* bnode = (block_node_t*)node;
            declaration_list_t* dae  = bnode->declaration_list;  // this declaration_list is freed when the whole ast is freed
                                                                 // NOTE: the ids will be freed with the ast (they were allocated with it, so they should be deallocated with it)

            // For each declaration in this scope create an association in this scope's evaluation environment
            for (int i = 0; i < dae->size; i++)
                // TODO .node could be NULL
                assoc(scope_env, dae->declarations[i].id, (union association_v){ .integer = eval(dae->declarations[i].node, scope_env)});

            int val = eval(((block_node_t*)node)->body, scope_env);

            endScope(scope_env); // free the scope environment and its association array

            return val;
        }

        case NUM:
            return ((num_node_t*)node)->value;

        case ADD:
            return eval(((binary_node_t*)node)->left, e) + eval(((binary_node_t*)node)->right, e);

        case SUB:
            return eval(((binary_node_t*)node)->left, e) - eval(((binary_node_t*)node)->right, e);

        case MUL:
            return eval(((binary_node_t*)node)->left, e) * eval(((binary_node_t*)node)->right, e);

        case DIV:
            return eval(((binary_node_t*)node)->left, e) / eval(((binary_node_t*)node)->right, e);

        case UMINUS:
            return - eval(((unary_node_t*)node)->child, e);

        default:
            fprintf(stderr, "ERROR: Undefined eval for operation %d\n!", node->type);
            exit(1);
    }
}

int main(int argc, char *argv[]) {

    printf("Input an expression: ");

    printf("Parsing...\n");
    node_t* root = parse_root();

    environment_t* topenv = newEnvironment();

    printf("Evaluating...\n");
    int val = eval(root, topenv);

    free(topenv);

    printf("Freeing AST...\n");
    free_ast(root);

    printf("Result: %d\n", val);

    return 0;
}

