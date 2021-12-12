/* #include <llvm-c/Core.h> */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "ast.h"
#include "parse_utils.h"
/* #include "llvm.h" */
#include <llvm-c/Core.h>

LLVMValueRef compile(LLVMBuilderRef b, node_t* node, environment_t* e) {
    switch (node->type) {

        /* case ID: */
        /*     return (int)(intptr_t)find(e, ((id_node_t*)node)->value); */

        /* case BLOCK: { */
        /*     environment_t* scope_env = beginScope(e); */
            
        /*     block_node_t* bnode = (block_node_t*)node; */
        /*     declaration_list_t* dae  = bnode->declaration_list;  // this id->ast_node environment is freed when the whole ast is freed */
        /*                                                         // NOTE: the ids will be freed with the ast (they were allocated with it, so they should be deallocated with it) */

        /*     // For each declaration in this scope create an association in this scope's evaluation environment */
        /*     for (int i = 0; i < dae->size; i++) */
        /*         // TODO .val could be NULL */
        /*         assoc(scope_env, dae->declarations[i].id, (void*)(intptr_t)eval(dae->declarations[i].node, scope_env)); */

        /*     int val = eval(((block_node_t*)node)->body, scope_env); */

        /*     endScope(scope_env); // free the scope environment and its association array */

        /*     return val; */
        /* } */

        case NUM:
            return LLVMConstInt(LLVMInt32Type(), ((num_node_t*)node)->value, 0 /* LLVMBool for SignExtend? TODO: What is SignExtend */);

        case ADD:
            return LLVMBuildAdd(b, compile(b, ((binary_node_t*)node)->left, e), compile(b, ((binary_node_t*)node)->right, e), "addtmp");

        case SUB:
            return LLVMBuildSub(b, compile(b, ((binary_node_t*)node)->left, e), compile(b, ((binary_node_t*)node)->right, e), "subtmp");

        case MUL:
            return LLVMBuildMul(b, compile(b, ((binary_node_t*)node)->left, e), compile(b, ((binary_node_t*)node)->right, e), "multmp");

        case DIV:
            return LLVMBuildSDiv(b, compile(b, ((binary_node_t*)node)->left, e), compile(b, ((binary_node_t*)node)->right, e), "sdivtmp");

        /* case UMINUS: */
        /*     return - eval(node->child, e); */

        default:
            fprintf(stderr, "ERROR: Undefined eval for operation %d\n!", node->type);
            exit(1);
            return NULL;
    }
}

/* LLVMContext* lc; */
/* Module* mod; */
/* IRBuilder* builder; */

int main(int argc, char *argv[]) {

    printf("Input an expression to compile then EOF (<C-d>): ");
    node_t* root = parse_root();

    LLVMModuleRef mod = LLVMModuleCreateWithName("llvm!"); // new, empty module in the global context
    LLVMBuilderRef builder = LLVMCreateBuilder();          // builder in the global context starting at entry

    /* LLVMBasicBlockRef entry = LLVMAppendBasicBlock(sum, "entry"); // basic block */
    /* LLVMPositionBuilderAtEnd(builder, entry); */

    environment_t* env = newEnvironment();

    LLVMValueRef c = compile(builder, root, env);

    free(env);

    printf("Result:\n%s\n", LLVMPrintValueToString(c)); 

    free_ast(root);

    LLVMDisposeBuilder(builder);
    LLVMDisposeModule(mod);

    return 0;
}
