#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/Target.h>
#include <llvm-c/Transforms/Scalar.h>
#include <llvm-c/Transforms/Utils.h>
#include "ast.h"
#include "parse_utils.h"
/* #include "llvm.h" */

#define NOOPTIMIZE

type_specifier_t get_larger_type(type_specifier_t l, type_specifier_t r) {

    // TODO: is the result of binary operation on numbers signed or unsigned?
    return (l & 0xf) < (r & 0xf) ? r : l;
}

type_specifier_t typecheck(node_t* node, environment_t* e) {
    type_specifier_t t;
    switch (node->type) {
        case FUNCTION: {
            // TODO ... 
            /* this doesn't need to be, only make sure that its castable from one to another assert((t = node->ts) == typecheck(((function_node_t*)node)->body, e)); */
            t = node->ts; // function type is its return value... quite wrong but ..
            break;
        }
        case ID: {
            t = (int)(intptr_t)find(e, ((id_node_t*)node)->value);
            node->ts = t; // Set this node's type to the one found in the environment
            break;
        }
        case BLOCK: {
             
            environment_t* scope_env = beginScope(e);
            
            block_node_t* bnode = (block_node_t*)node;
            declaration_list_t* dae = bnode->declaration_list;  // this id->ast_node environment is freed when the whole ast is freed
                                                                // NOTE: the ids will be freed with the ast (they were allocated with it, so they should be deallocated with it)

            // For each declaration in this scope create an association in this scope's evaluation environment
            for (int i = 0; i < dae->size; i++) {
                dae->declarations[i].node->ts = dae->declarations[i].ds.ts;
                assoc(scope_env, dae->declarations[i].id, (void*)(intptr_t)typecheck(dae->declarations[i].node, scope_env));
            }

            t = typecheck(((block_node_t*)node)->body, scope_env);

            endScope(scope_env); // free the scope environment and its association array

            break;
        }
        case NUM: {
            // TODO: Min required value to hold this number?
            t = INT;
            break;
        }
        case ADD:
        case SUB:
        case MUL:
        case DIV: {
            type_specifier_t l = typecheck(((binary_node_t*)node)->left, e);
            type_specifier_t r = typecheck(((binary_node_t*)node)->right, e);
            // TODO: assert both types are "addable"... (aren't they all?)
            t = get_larger_type(l, r);
            break;
        }
        case UMINUS: {
            t = typecheck(((unary_node_t*)node)->child, e);
            // TODO: assert type is numeric...
            break;
        }
    }
    return t;
}

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

LLVMTypeRef type2LLVMType(type_specifier_t ts) {

    switch (ts) {
        case INT:
            return LLVMInt32Type();
        case SHORT:
            return LLVMInt16Type();
        case CHAR:
            return LLVMInt8Type();
    }
}

struct LLVMValueRefPair {
    LLVMValueRef left, right;
};

struct LLVMValueRefPair sextBinaryIntOpOperands(LLVMBuilderRef b, LLVMValueRef left, LLVMValueRef right) {

    LLVMTypeRef lt = LLVMTypeOf(left);
    LLVMTypeRef rt = LLVMTypeOf(right);
    unsigned lis = LLVMGetIntTypeWidth(lt);  // left int size
    unsigned ris = LLVMGetIntTypeWidth(rt); // right int size

    // TODO: extension depends on whether number is signed or unsigned
    if (lis < ris)
        // TODO: Como ter o tipo do nó compilado sem ser com LLVM?
        left = (is_int_type_unsigned(CHAR) ? LLVMBuildZExt : LLVMBuildSExt)(b, left, rt, "sextlefttmp"); // Sign extend left type to match right type size
    else if (lis > ris)
        right = (is_int_type_unsigned(CHAR) ? LLVMBuildZExt : LLVMBuildSExt)(b, right, lt, "sextrighttmp"); // Sign extend right type to match left type size

    return (struct LLVMValueRefPair){ left, right };
}

LLVMValueRef compile(LLVMModuleRef m, LLVMBuilderRef b, node_t* node, environment_t* e) {
    switch (node->type) {

        case FUNCTION: {
            LLVMTypeRef fun_type = type2LLVMType(node->ts);
            LLVMTypeRef ftype = LLVMFunctionType(fun_type, NULL, 0, 0);
            LLVMValueRef fun = LLVMAddFunction(m, ((function_node_t*)node)->name, ftype);
            LLVMBasicBlockRef entry = LLVMAppendBasicBlock(fun, "entry");
            LLVMPositionBuilderAtEnd(b, entry);
            // TODO: arguments environment
            LLVMValueRef body_value = compile(m, b, ((function_node_t*)node)->body, e);
            LLVMTypeRef body_type = LLVMTypeOf(body_value);

            if (body_type != fun_type) // body type and function return type are different
                // if they both are ints, truncate or extend return value
                if (LLVMGetTypeKind(body_type) == LLVMIntegerTypeKind && LLVMGetTypeKind(fun_type) == LLVMIntegerTypeKind) {
                    if (LLVMGetIntTypeWidth(body_type) < LLVMGetIntTypeWidth(fun_type))
                        body_value = LLVMBuildZExt(b, body_value, fun_type, "sexttmp"); // TODO: SExt?
                    else
                        body_value = LLVMBuildTrunc(b, body_value, fun_type, "trunctmp");
                }

            LLVMBuildRet(b, body_value);

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
                // TODO .val could be NULL?
                LLVMValueRef alloca = LLVMBuildAlloca(b, type2LLVMType(dae->declarations[i].ds.ts), "allocatmp");
                LLVMBuildStore(b, compile(m, b, dae->declarations[i].node, scope_env), alloca);
                assoc(scope_env, dae->declarations[i].id, alloca);
            }

            LLVMValueRef val = compile(m, b, ((block_node_t*)node)->body, scope_env);

            endScope(scope_env); // free the scope environment and its association array

            return val;
        }

        case NUM: { 
            return LLVMConstInt(type2LLVMType(node->ts), ((num_node_t*)node)->value, node->ts == CHAR ? 0 : 1 /* TODO: reunderstand sign extension */ );
        }

        case ADD: {
            struct LLVMValueRefPair vpair = sextBinaryIntOpOperands(b,
                    compile(m, b, ((binary_node_t*)node)->left, e),
                    compile(m, b, ((binary_node_t*)node)->right, e));
            return LLVMBuildAdd(b, vpair.left, vpair.right, "addtmp");
        }

        case SUB: {
            struct LLVMValueRefPair vpair = sextBinaryIntOpOperands(b,
                    compile(m, b, ((binary_node_t*)node)->left, e),
                    compile(m, b, ((binary_node_t*)node)->right, e));
            return LLVMBuildSub(b, vpair.left, vpair.right, "subtmp");
        }

        case MUL: {
            struct LLVMValueRefPair vpair = sextBinaryIntOpOperands(b,
                    compile(m, b, ((binary_node_t*)node)->left, e),
                    compile(m, b, ((binary_node_t*)node)->right, e));
            return LLVMBuildMul(b, vpair.left, vpair.right, "multmp");
        }

        case DIV: {
            struct LLVMValueRefPair vpair = sextBinaryIntOpOperands(b,
                    compile(m, b, ((binary_node_t*)node)->left, e),
                    compile(m, b, ((binary_node_t*)node)->right, e));
            return LLVMBuildSDiv(b, vpair.left, vpair.right, "sdivtmp");
        }

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

    printf("[ Type Checking ]\n");
    environment_t* typing_env = newEnvironment();
    typecheck(root, typing_env);
    free(typing_env);

    printf("[ Setup ]\n");
    LLVMModuleRef module = LLVMModuleCreateWithName("llvm!"); // new, empty module in the global context
    LLVMBuilderRef builder = LLVMCreateBuilder();          // builder in the global context starting at entry

#ifdef OPTIMIZE
    create_llvm_pass_manager(module); // Initialize global pass_manager
#endif

    environment_t* env = newEnvironment();

    printf("[ Compiling ]\n");
    compile(module, builder, root, env);

    printf("[ Cleaning ]\n");
    free(env);
    free_ast(root);

    printf("[ Printing ]\n");
    char* module_string = LLVMPrintModuleToString(module);
    printf("[ Module ]\n%s", module_string);

    char *error = NULL;
    LLVMVerifyModule(module, LLVMAbortProcessAction, &error);
    LLVMDisposeMessage(error);

    LLVMDisposeBuilder(builder);
    LLVMDisposeModule(module);

#ifdef OPTIMIZE
    LLVMDisposePassManager(pass_manager);
#endif

    return 0;
}
