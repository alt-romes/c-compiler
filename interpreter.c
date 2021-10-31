#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "environment.h"
#include "parse_utils.h"
#include "llvm.h"

int eval(const node_t* node, environment_t* e) {
    switch (node->type) {

        case ID:
            return (int)(intptr_t)find(e, node->id_value);

        case DEF: {
            environment_t* scope_env = beginScope(e);
            assoc(scope_env, node->def.id, (void*)(intptr_t)eval(node->def.left, e));
            return eval(node->def.right, scope_env);
        }

        case NUM:
            return node->num_value;

        case ADD:
            return eval(node->children.left, e) + eval(node->children.right, e);

        case SUB:
            return eval(node->children.left, e) - eval(node->children.right, e);

        case MUL:
            return eval(node->children.left, e) * eval(node->children.right, e);

        case DIV:
            return eval(node->children.left, e) / eval(node->children.right, e);

        case UMINUS:
            return - eval(node->child, e);

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

    print_int(initialize_context(), 16);

    return 0;
}
