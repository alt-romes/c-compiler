#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "ast.h"
#include "parse_utils.h"
#include "dcpuIR.h"

void compile_dcpu(node_t* node) {

    switch (node->type) {

        case NUM:
            emit_num(((num_node_t*)node)->value);
            break;

        case ADD:
            compile_dcpu(((binary_node_t*)node)->left);
            compile_dcpu(((binary_node_t*)node)->right);
            emit_add();
            break;

        case SUB:
            compile_dcpu(((binary_node_t*)node)->left);
            compile_dcpu(((binary_node_t*)node)->right);
            emit_sub();
            break;

        case MUL:
            compile_dcpu(((binary_node_t*)node)->left);
            compile_dcpu(((binary_node_t*)node)->right);
            emit_mul();
            break;

        //case DIV:

        case UMINUS:
            compile_dcpu(((unary_node_t*)node)->child);
            emit_uminus();
            break;

        default:
            fprintf(stderr, "ERROR: Undefined eval for operation %d\n!", node->type);
            exit(1);
            //return NULL;
    }
}

int main(int argc, char *argv[]) {

    printf("Input an expression to compile: ");
    node_t* root = parse_root();

    printf("Result:\n");

    compile_dcpu(root);
    dcpu_print();
    dcpu_free();

    free_ast(root);

    return 0;
}
