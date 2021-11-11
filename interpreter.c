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
            environment_t* dae  = bnode->declarations_ast_env; // this id->ast_node environment is freed when the whole ast is freed

            // For each declaration in this scope create an association in this scope's evaluation environment
            for (int i = 0; i < dae->size; i++)
                // TODO .val could be NULL
                assoc(scope_env, dae->associations[i].id, (void*)(intptr_t)eval(dae->associations[i].val, e));

            // TODO free scope_env?
            return eval(((block_node_t*)node)->body, scope_env);
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

    int val = eval(root, newEnvironment());
    // TODO free this newEnv

    printf("Result: %d\n", val);

    free_ast(root);

    return 0;
}

