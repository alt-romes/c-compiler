#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/Target.h>
#include <llvm-c/Transforms/Scalar.h>
#include <llvm-c/Transforms/Utils.h>
#include "llvm.h"
#include "ast.h"
#include "parse_utils.h"

#define NOOPTIMIZE

#ifdef OPTIMIZE
LLVMPassManagerRef pass_manager; // Define as global because so much parameter passing is a write-time and run-time overhead :P

void create_llvm_pass_manager(LLVMModuleRef module) {
    pass_manager = LLVMCreateFunctionPassManagerForModule(module);

    /* LLVMAddTargetData(LLVMGetExecutionEngineTargetData(engine), pass_manager); */
    LLVMAddPromoteMemoryToRegisterPass(pass_manager);
    LLVMAddInstructionCombiningPass(pass_manager);
    LLVMAddReassociatePass(pass_manager);
    LLVMAddGVNPass(pass_manager);
    LLVMAddCFGSimplificationPass(pass_manager);
    LLVMInitializeFunctionPassManager(pass_manager);
}
#endif

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

            LLVMVerifyFunction(fun, LLVMAbortProcessAction);

#ifdef OPTIMIZE
            // Run optimizations!
            LLVMRunFunctionPassManager(pass_manager, fun);
#endif

            return fun;
        }

        case ID: {
            LLVMValueRef alloca = (LLVMValueRef)find(e, ((id_node_t*)node)->value);
            return LLVMBuildLoad2(b, LLVMGetAllocatedType(alloca), alloca, "loadtmp");
        }

        case BLOCK: {
            environment_t* scope_env = beginScope(e);
            
            block_node_t* bnode = (block_node_t*)node;
            declaration_list_t* dae  = bnode->declaration_list;  // this id->ast_node environment is freed when the whole ast is freed
                                                                // NOTE: the ids will be freed with the ast (they were allocated with it, so they should be deallocated with it)

            // For each declaration in this scope create an association in this scope's evaluation environment
            for (int i = 0; i < dae->size; i++) {
                // TODO .val could be NULL
                LLVMValueRef alloca = LLVMBuildAlloca(b, LLVMInt32Type() /* TODO: Use declaration type */, "allocatmp");
                LLVMBuildStore(b, compile(m, b, dae->declarations[i].node, scope_env), alloca);
                assoc(scope_env, dae->declarations[i].id, alloca);
            }

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
    LLVMModuleRef module = LLVMModuleCreateWithName("llvm!"); // new, empty module in the global context
    LLVMBuilderRef builder = LLVMCreateBuilder();          // builder in the global context starting at entry

#ifdef OPTIMIZE
    create_llvm_pass_manager(module); // Initialize global pass_manager
#endif

    environment_t* env = newEnvironment();

    printf("[ Compiling ]\n");
    LLVMValueRef compiled = compile(module, builder, root, env);

    printf("[ Cleaning ]\n");
    free(env);
    free_ast(root);

    /* printf("%s", LLVMPrintValueToString(compiled)); */ 

    printf("[ Printing ]\n");
    LLVMDumpModule(module);

    char *error = NULL;
    LLVMVerifyModule(module, LLVMAbortProcessAction, &error);
    LLVMDisposeMessage(error);

    LLVMDisposeBuilder(builder);
    LLVMDisposeModule(module);

#ifdef OPTIMIZE
    LLVMDisposePassManager(pass_manager);
#endif

    /* llvm_optimize(mod); */

    return 0;
}
