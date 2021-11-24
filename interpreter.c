#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "ast.h"
#include "environment.h"
#include "parse_utils.h"

int eval(const node_t* node, environment_t* e) {
    switch (node->type) {

        case ID:
            return (int)(intptr_t)find(e, ((id_node_t*)node)->value);

        case BLOCK: {
            environment_t* scope_env = beginScope(e);
            
            block_node_t* bnode = (block_node_t*)node;
            environment_t* dae  = bnode->declarations_ast_env;  // this id->ast_node environment is freed when the whole ast is freed
                                                                // NOTE: the ids will be freed with the ast (they were allocated with it, so they should be deallocated with it)

            // For each declaration in this scope create an association in this scope's evaluation environment
            for (int i = 0; i < dae->size; i++)
                // TODO .val could be NULL
                assoc(scope_env, dae->associations[i].id, (void*)(intptr_t)eval(dae->associations[i].val, scope_env));

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

    node_t* root = parse_root();

    environment_t* topenv = newEnvironment();

    int val = eval(root, topenv);

    free(topenv);

    free_ast(root);

    printf("Result: %d\n", val);

    return 0;
}

