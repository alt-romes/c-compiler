/* #include <llvm-c/Core.h> */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "ast.h"
#include "parse_utils.h"
/* #include "llvm.h" */
#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>

LLVMValueRef compile(LLVMModuleRef m, LLVMBuilderRef b, node_t* node, environment_t* e) {
    switch (node->type) {

        case FUNCTION: {
            // TODO: Add return type from AST
            LLVMTypeRef ftype = LLVMFunctionType(LLVMInt32Type(), NULL, 0, 0);
            LLVMValueRef fun = LLVMAddFunction(m, ((function_node_t*)node)->name, ftype);
            LLVMBasicBlockRef entry = LLVMAppendBasicBlock(fun, "entry");
            LLVMPositionBuilderAtEnd(b, entry);
            // TODO: arguments environment
            LLVMBuildRet(b, compile(m, b, ((function_node_t*)node)->body, e));

            return fun;
        }

        case ID:
            return (LLVMValueRef)find(e, ((id_node_t*)node)->value);

        case BLOCK: {
            environment_t* scope_env = beginScope(e);
            
            block_node_t* bnode = (block_node_t*)node;
            declaration_list_t* dae  = bnode->declaration_list;  // this id->ast_node environment is freed when the whole ast is freed
                                                                // NOTE: the ids will be freed with the ast (they were allocated with it, so they should be deallocated with it)

            // For each declaration in this scope create an association in this scope's evaluation environment
            for (int i = 0; i < dae->size; i++)
                // TODO .val could be NULL
                assoc(scope_env, dae->declarations[i].id, compile(m, b, dae->declarations[i].node, scope_env));

            LLVMValueRef val = compile(m, b, ((block_node_t*)node)->body, scope_env);

            endScope(scope_env); // free the scope environment and its association array

            return val;
        }

        case NUM:
            return LLVMConstInt(LLVMInt32Type(), ((num_node_t*)node)->value, 0 /* LLVMBool for SignExtend? TODO: What is SignExtend */);

        case ADD:
            return LLVMBuildAdd(b, compile(m, b, ((binary_node_t*)node)->left, e), compile(m, b, ((binary_node_t*)node)->right, e), "addtmp");

        case SUB:
            return LLVMBuildSub(b, compile(m, b, ((binary_node_t*)node)->left, e), compile(m, b, ((binary_node_t*)node)->right, e), "subtmp");

        case MUL:
            return LLVMBuildMul(b, compile(m, b, ((binary_node_t*)node)->left, e), compile(m, b, ((binary_node_t*)node)->right, e), "multmp");

        case DIV:
            return LLVMBuildSDiv(b, compile(m, b, ((binary_node_t*)node)->left, e), compile(m, b, ((binary_node_t*)node)->right, e), "sdivtmp");

        case UMINUS:
            return LLVMBuildNeg(b, compile(m, b, ((unary_node_t*)node)->child, e), "negtmp");

        default:
            fprintf(stderr, "ERROR: Undefined eval for operation %d\n!", node->type);
            exit(1);
            return NULL;
    }
}

int main(int argc, char *argv[]) {

    printf("[ Parsing ]\n");
    node_t* root = parse_root();

    printf("[ Setup ]\n");
    LLVMModuleRef mod = LLVMModuleCreateWithName("llvm!"); // new, empty module in the global context
    LLVMBuilderRef builder = LLVMCreateBuilder();          // builder in the global context starting at entry

    environment_t* env = newEnvironment();

    printf("[ Compiling ]\n");
    LLVMValueRef c = compile(mod, builder, root, env);

    free(env);

    printf("%s", LLVMPrintValueToString(c)); 

    printf("[ Cleaning ]\n");
    free_ast(root);

    char *error = NULL;
    LLVMVerifyModule(mod, LLVMAbortProcessAction, &error);
    LLVMDisposeMessage(error);

    LLVMDisposeBuilder(builder);
    LLVMDisposeModule(mod);

    return 0;
}
