/* #include <llvm-c/Core.h> */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "ast.h"
#include "parse_utils.h"
#include "llvm.h"

LLVMValue* compile(LLVMContext* lc, IRBuilder* b, node_t* node) {
    switch (node->type) {

        case ID:
            return NULL;
            /* return (int)(intptr_t)find(e, node->id_value); */

        /* case DEF: { */
        /*     environment_t* scope_env = beginScope(e); */
        /*     assoc(scope_env, node->def.id, (void*)(intptr_t)eval(node->def.left, e)); */
        /*     return eval(node->def.right, scope_env); */
        /* } */

        case NUM:
            return constant_int(lc, ((num_node_t*)node)->value);

        case ADD:
            return build_add(b, compile(lc, b, ((binary_node_t*)node)->left), compile(lc, b, ((binary_node_t*)node)->right));

        case SUB:
            return build_sub(b, compile(lc, b, ((binary_node_t*)node)->left), compile(lc, b, ((binary_node_t*)node)->right));

        case MUL:
            return build_mul(b, compile(lc, b, ((binary_node_t*)node)->left), compile(lc, b, ((binary_node_t*)node)->right));

        case DIV:
            return build_div(b, compile(lc, b, ((binary_node_t*)node)->left), compile(lc, b, ((binary_node_t*)node)->right));

        /* case UMINUS: */
        /*     return - eval(node->child, e); */

        default:
            fprintf(stderr, "ERROR: Undefined eval for operation %d\n!", node->type);
            exit(1);
            return NULL;
    }
}

LLVMContext* lc;
Module* mod;
IRBuilder* builder;

int main(int argc, char *argv[]) {

    printf("Input an expression to compile: ");
    node_t* root = parse_root();

    lc = initialize_context();
    mod = initialize_module("llvm!", lc);
    builder = initialize_builder(lc);

    printf("Result:\n");

    LLVMValue* c = compile(lc, builder, root);

    print_llvmvalue(c);

    printf("\n");

    free_ast(root);

    free_llvm((void*)lc);
    free_llvm((void*)mod);
    free_llvm((void*)builder);

    return 0;
}
