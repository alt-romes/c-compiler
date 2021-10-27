#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "parse_utils.h"

int eval(const node_t* node) {
    switch (node->type) {
        case NUM:
            return node->num_value;

        case ADD:
            return eval(node->children.left) + eval(node->children.right);

        case SUB:
            return eval(node->children.left) - eval(node->children.right);

        case MUL:
            return eval(node->children.left) * eval(node->children.right);

        case DIV:
            return eval(node->children.left) / eval(node->children.right);

        case UMINUS:
            return - eval(node->child);

        default:
            fprintf(stderr, "ERROR: Undefined eval for operation %d\n!", node->type);
            exit(1);
    }
}

int main(int argc, char *argv[]) {

    printf("Input an expression: ");

    node_t* root = parse_root();

    int val = eval(root);

    printf("Result: %d\n", val);

    free_ast(root);

    return 0;
}
