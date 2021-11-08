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

        case DEF: {
            environment_t* scope_env = beginScope(e);
            assoc(scope_env, ((def_node_t*)node)->id, (void*)(intptr_t)eval(((def_node_t*)node)->left, e));
            return eval(((def_node_t*)node)->right, scope_env);
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

    printf("Result: %d\n", val);

    free_ast(root);

    return 0;
}

