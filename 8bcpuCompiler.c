#include <stdlib.h>
#include <stdio.h>
#include "ast.h"
#include "parse_utils.h"

void compile8bcpu(node_t* node) {

    switch (node->type) {

        case NUM:
            printf("psh $%d\n", node->num_value);

        case ADD:
            compile8bcpu(node->children.left);
            compile8bcpu(node->children.right);
            printf("pop RB\n");
            printf("pop RC\n");
            printf("add RB RC\n");
            printf("lod ACR RB\n");
            printf("psh RB\n");

        case SUB:
            compile8bcpu(node->children.left);
            compile8bcpu(node->children.right);
            printf("pop RB\n");
            printf("pop RC\n");
            printf("sub RB RC\n");
            printf("lod ACR RB\n");
            printf("psh RB\n");
        case MUL:
            compile8bcpu(node->children.left);
            compile8bcpu(node->children.right);
            printf("pop RB\n"); // a
            printf("pop RC\n"); // b

            printf("lod $0 RD\n"); // res
            printf("lod $0 RE\n"); // i
            printf("loop_start:\n");
            printf("add RD RB\n"); // res += a
            printf("lod ACR RD\n");
            printf("inc RE\n"); // i++
            printf("lod ACR RE\n");
            printf("sub RE RC\n"); // if i < b goto loop_start
            printf("jmpn loop_start\n");

            printf("psh RD\n");

        //case DIV:

        case UMINUS:
            compile8bcpu(node->child);
            printf("pop RB\n");
            printf("neg RB\n");
            printf("psh RB\n");

        default:
            fprintf(stderr, "ERROR: Undefined eval for operation %d\n!", node->type);
            exit(1);
            //return NULL;
    }
}

int main(int argc, char *argv[]) {

    printf("Input an expression to compile8bcpu: ");
    node_t* root = parse_root();


    printf("Result:\n");

    compile8bcpu(root);
    return 0;
}
