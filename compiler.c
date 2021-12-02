/* #include <llvm-c/Core.h> */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "ast.h"
#include "parse_utils.h"
#include "llvm.h"

LLVMValue* compile(LLVMContext* lc, IRBuilder* b, node_t* node, environment_t* e) {
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
            return constant_int(lc, ((num_node_t*)node)->value);

        case ADD:
            return build_add(b, compile(lc, b, ((binary_node_t*)node)->left, e), compile(lc, b, ((binary_node_t*)node)->right, e));

        case SUB:
            return build_sub(b, compile(lc, b, ((binary_node_t*)node)->left, e), compile(lc, b, ((binary_node_t*)node)->right, e));

        case MUL:
            return build_mul(b, compile(lc, b, ((binary_node_t*)node)->left, e), compile(lc, b, ((binary_node_t*)node)->right, e));

        case DIV:
            return build_div(b, compile(lc, b, ((binary_node_t*)node)->left, e), compile(lc, b, ((binary_node_t*)node)->right, e));

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

    printf("Input an expression to compile then EOF (<C-d>): ");
    node_t* root = parse_root();

    lc = initialize_context();
    mod = initialize_module("llvm!", lc);
    builder = initialize_builder(lc);

    printf("Result:\n");

    environment_t* env = newEnvironment();

    LLVMValue* c = compile(lc, builder, root, env);

    free(env);

    print_llvmvalue(c);

    printf("\n");

    free_ast(root);

    free_llvm((void*)lc);
    free_llvm((void*)mod);
    free_llvm((void*)builder);

    return 0;
}
