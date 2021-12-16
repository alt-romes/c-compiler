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
#include "typecheck.h"
#include "parse_utils.h"
/* #include "llvm.h" */

#define OPTIMIZE

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

LLVMTypeRef type2LLVMType(enum type ts) {

    switch (ts & 0xf) {
        case INT:
            return LLVMInt32Type();
        case SHORT:
            return LLVMInt16Type();
        case CHAR:
            return LLVMInt8Type();
        default:
            fprintf(stderr, "type2LLVM undefined for ts: %d\n", ts);
            exit(1);
    }

}

struct LLVMValueRefPair {
    LLVMValueRef left, right;
};

struct LLVMValueRefPair ext_int_binaryop_operands(LLVMBuilderRef b, enum type lnt, LLVMValueRef left, enum type rnt, LLVMValueRef right) {

    LLVMTypeRef lt = LLVMTypeOf(left);
    LLVMTypeRef rt = LLVMTypeOf(right);

    printf("comparing type %d with %d\n", lnt, rnt);

    if (type_compare(lnt, rnt) < 0) { // left is smaller than right
        left = (is_int_type_unsigned(lnt) ? LLVMBuildZExt : LLVMBuildSExt)(b, left, rt, "sextlefttmp"); // Extend left type to match right type size
        printf("left node is smaller\n");
    }
    else if (type_compare(lnt, rnt) > 0) { // left is larger than right
        right = (is_int_type_unsigned(rnt) ? LLVMBuildZExt : LLVMBuildSExt)(b, right, lt, "sextrighttmp"); // Extend right type to match left type size
        printf("left node is larger\n");
    }

    return (struct LLVMValueRefPair){ left, right };
}

LLVMValueRef ext_or_trunc(LLVMBuilderRef b, enum type dst_type, enum type src_type, LLVMValueRef src) {

    if (type_compare(src_type, dst_type) < 0)
        src = (is_int_type_unsigned(src_type) ? LLVMBuildZExt : LLVMBuildSExt)(b, src, type2LLVMType(dst_type), "extsrctmp");
    else if (type_compare(src_type, dst_type) > 0)
        src = LLVMBuildTrunc(b, src, type2LLVMType(dst_type), "truncsrctmp"); // extend right type to match left type size

    return src;
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

            if (LLVMTypeOf(body_value) != fun_type) {// body type and function return type are different

                if (LLVMGetTypeKind(LLVMTypeOf(body_value)) == LLVMIntegerTypeKind && // if they both are ints,
                    LLVMGetTypeKind(fun_type) == LLVMIntegerTypeKind) {  // truncate or extend return value

                    printf("fun type %d body type %d\n", node->ts, ((function_node_t*)node)->body->ts);
                    body_value = ext_or_trunc(b, node->ts, ((function_node_t*)node)->body->ts, body_value);
                }
                else {
                    fprintf(stderr, "Can't cast body value to return type:: TODO Move to typecheck\n");
                    exit(2);
                }
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
            LLVMValueRef alloca = find(e, ((id_node_t*)node)->value).llvmref;
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
                LLVMValueRef alloca = LLVMBuildAlloca(b, type2LLVMType(dae->declarations[i].et), "allocatmp");
                LLVMValueRef assignment_val = compile(m, b, dae->declarations[i].node, scope_env);
                LLVMBuildStore(b, ext_or_trunc(b, dae->declarations[i].et, dae->declarations[i].node->ts, assignment_val), alloca);
                assoc(scope_env, dae->declarations[i].id, (union association_v){ .llvmref = alloca });
            }

            LLVMValueRef val = compile(m, b, ((block_node_t*)node)->body, scope_env);

            endScope(scope_env); // free the scope environment and its association array

            return val;
        }

        case NUM: { 
            return LLVMConstInt(type2LLVMType(node->ts), ((num_node_t*)node)->value, node->ts == CHAR ? 0 : 1 /* TODO: reunderstand sign extension */ );
        }

        case ADD: {
            printf("Adding %d with %d\n", ((binary_node_t*)node)->left->ts, ((binary_node_t*)node)->right->ts);
            struct LLVMValueRefPair vpair = ext_int_binaryop_operands(b,
                    ((binary_node_t*)node)->left->ts,
                    compile(m, b, ((binary_node_t*)node)->left, e),
                    ((binary_node_t*)node)->right->ts,
                    compile(m, b, ((binary_node_t*)node)->right, e));
            return LLVMBuildAdd(b, vpair.left, vpair.right, "addtmp");
        }

        case SUB: {
            struct LLVMValueRefPair vpair = ext_int_binaryop_operands(b,
                    ((binary_node_t*)node)->left->ts,
                    compile(m, b, ((binary_node_t*)node)->left, e),
                    ((binary_node_t*)node)->right->ts,
                    compile(m, b, ((binary_node_t*)node)->right, e));
            return LLVMBuildSub(b, vpair.left, vpair.right, "subtmp");
        }

        case MUL: {
            struct LLVMValueRefPair vpair = ext_int_binaryop_operands(b,
                    ((binary_node_t*)node)->left->ts,
                    compile(m, b, ((binary_node_t*)node)->left, e),
                    ((binary_node_t*)node)->right->ts,
                    compile(m, b, ((binary_node_t*)node)->right, e));
            return LLVMBuildMul(b, vpair.left, vpair.right, "multmp");
        }

        case DIV: {
            struct LLVMValueRefPair vpair = ext_int_binaryop_operands(b,
                    ((binary_node_t*)node)->left->ts,
                    compile(m, b, ((binary_node_t*)node)->left, e),
                    ((binary_node_t*)node)->right->ts,
                    compile(m, b, ((binary_node_t*)node)->right, e));
            // TODO: Depends on sign
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
