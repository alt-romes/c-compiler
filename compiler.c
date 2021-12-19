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
        case I1:
            return LLVMIntType(1);
        case LONG:
            return LLVMInt64Type();
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

    if (type_compare(lnt, rnt) < 0) // left is smaller than right
        left = (is_int_type_unsigned(lnt) ? LLVMBuildZExt : LLVMBuildSExt)(b, left, rt, "sextlefttmp"); // Extend left type to match right type size
    else if (type_compare(lnt, rnt) > 0) // left is larger than right
        right = (is_int_type_unsigned(rnt) ? LLVMBuildZExt : LLVMBuildSExt)(b, right, lt, "sextrighttmp"); // Extend right type to match left type size

    return (struct LLVMValueRefPair){ left, right };
}

LLVMValueRef ext_or_trunc(LLVMBuilderRef b, enum type dst_type, enum type src_type, LLVMValueRef src) {

    if (type_compare(src_type, dst_type) < 0)
        src = (is_int_type_unsigned(src_type) ? LLVMBuildZExt : LLVMBuildSExt)(b, src, type2LLVMType(dst_type), "extsrctmp");
    else if (type_compare(src_type, dst_type) > 0)
        src = LLVMBuildTrunc(b, src, type2LLVMType(dst_type), "truncsrctmp"); // extend right type to match left type size

    return src;
}

/* 
 * Transform an integer number into a boolean as an llvm i1 register value
 * @b -> Instruction builder
 * @a -> Value to convert to boolean, value == 0 => false (i1=0) ^ value != 1 => true (i1=1)
 * @a_type -> Int type size of a value
 */
LLVMValueRef llvmInt2BoolI1(LLVMBuilderRef b, LLVMValueRef a, enum type a_type) {
    return LLVMBuildNot(b, LLVMBuildICmp(b, LLVMIntEQ, a, LLVMConstInt(type2LLVMType(a_type), 0, 0), "tobooli1tmp"), "tobooli1nottmp");
}


LLVMValueRef compile(LLVMModuleRef m, LLVMBuilderRef b, node_t* node, environment_t* e) {

    union {
        LLVMIntPredicate llvmIntPredicate;
    } aux;

    aux.llvmIntPredicate = 0;

    switch (node->type) {

        case FUNCTION: {
            LLVMTypeRef fun_type = type2LLVMType(node->ts);
            LLVMTypeRef ftype = LLVMFunctionType(fun_type, NULL, 0, 0);
            LLVMValueRef fun = LLVMAddFunction(m, ((function_node_t*)node)->name, ftype);
            LLVMBasicBlockRef entry = LLVMAppendBasicBlock(fun, "entry");
            LLVMPositionBuilderAtEnd(b, entry);
            // TODO: arguments environment
            LLVMValueRef body_value = compile(m, b, ((function_node_t*)node)->body, e);

            if (((function_node_t*)node)->body->ts != node->ts) { // body type and function return type are different

                if (LLVMGetTypeKind(LLVMTypeOf(body_value)) == LLVMIntegerTypeKind && // if they both are ints,
                    LLVMGetTypeKind(fun_type) == LLVMIntegerTypeKind) {  // truncate or extend return value

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
                LLVMBuildStore(b, ext_or_trunc(b, dae->declarations[i].et, dae->declarations[i].node->ts, assignment_val) /* cast value to id type */, alloca);
                assoc(scope_env, dae->declarations[i].id, (union association_v){ .llvmref = alloca });
            }

            LLVMValueRef val = compile(m, b, ((block_node_t*)node)->body, scope_env);

            endScope(scope_env); // free the scope environment and its association array

            return val;
        }

        case NUM: { 
            return LLVMConstInt(type2LLVMType(node->ts), ((num_node_t*)node)->value, is_int_type_unsigned(node->ts) ? 0 : 1);
        }

        case ADD: {
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
            return (is_int_type_unsigned(node->ts)?LLVMBuildUDiv:LLVMBuildSDiv)(b, vpair.left, vpair.right, "sdivtmp");
        }

        // LT, GT, LE, and GE will have type unsigned if the comparison is between unsigned types, and signed type otherwise
        case LT:
            aux.llvmIntPredicate = aux.llvmIntPredicate == 0 ? (node->ts & UNSIGNED) ? LLVMIntULT : LLVMIntSLT : aux.llvmIntPredicate; 
        case GT:
            aux.llvmIntPredicate = aux.llvmIntPredicate == 0 ? (node->ts & UNSIGNED) ? LLVMIntUGT : LLVMIntSGT : aux.llvmIntPredicate; 
        case LE:
            aux.llvmIntPredicate = aux.llvmIntPredicate == 0 ? (node->ts & UNSIGNED) ? LLVMIntULE : LLVMIntSLE : aux.llvmIntPredicate; 
        case GE:
            aux.llvmIntPredicate = aux.llvmIntPredicate == 0 ? (node->ts & UNSIGNED) ? LLVMIntUGE : LLVMIntSGE : aux.llvmIntPredicate; 
        case NE:
            aux.llvmIntPredicate = aux.llvmIntPredicate == 0 ? LLVMIntNE : aux.llvmIntPredicate; 
        case EQ: {
            aux.llvmIntPredicate = aux.llvmIntPredicate == 0 ? LLVMIntEQ : aux.llvmIntPredicate; 

            struct LLVMValueRefPair vpair = ext_int_binaryop_operands(b,
                    ((binary_node_t*)node)->left->ts,
                    compile(m, b, ((binary_node_t*)node)->left, e),
                    ((binary_node_t*)node)->right->ts,
                    compile(m, b, ((binary_node_t*)node)->right, e));
            // Return boolean extended to relational operation type size (CHAR) based on auxiliary LLVMIntPredicate
            return LLVMBuildICmp(b, aux.llvmIntPredicate, vpair.left, vpair.right , "cmptmp");
        }

        // Booleans are represented in an i1 LLVM register, we can convert a number to an i1 boolean, and bit OR, AND, NOT operations on 1 bit are the same as logical operations
        case LOR:
            return LLVMBuildOr(b,
                    llvmInt2BoolI1(b,
                        compile(m, b, ((binary_node_t*)node)->left, e),
                        ((binary_node_t*)node)->left->ts),
                    llvmInt2BoolI1(b,
                        compile(m, b, ((binary_node_t*)node)->right, e),
                        ((binary_node_t*)node)->right->ts),
                    "ortmp");
        case LAND:
            return LLVMBuildAnd(b,
                    llvmInt2BoolI1(b,
                        compile(m, b, ((binary_node_t*)node)->left, e),
                        ((binary_node_t*)node)->left->ts),
                    llvmInt2BoolI1(b,
                        compile(m, b, ((binary_node_t*)node)->right, e),
                        ((binary_node_t*)node)->right->ts),
                    "andtmp");

        case UMINUS:
            return LLVMBuildNeg(b, compile(m, b, ((unary_node_t*)node)->child, e), "negtmp");

        case LOGICAL_NOT:
            // Possibly innefficient converting numbers to i1 booleans before doing boolean operations
            return LLVMBuildNot(b, llvmInt2BoolI1(b, compile(m, b, ((unary_node_t*)node)->child, e), ((unary_node_t*)node)->child->ts), "logicalnottmp");

        case BOR: {
            struct LLVMValueRefPair vpair = ext_int_binaryop_operands(b,
                    ((binary_node_t*)node)->left->ts,
                    compile(m, b, ((binary_node_t*)node)->left, e),
                    ((binary_node_t*)node)->right->ts,
                    compile(m, b, ((binary_node_t*)node)->right, e));
            return LLVMBuildOr(b, vpair.left, vpair.right, "multmp");
        }
        case BXOR: {
            struct LLVMValueRefPair vpair = ext_int_binaryop_operands(b,
                    ((binary_node_t*)node)->left->ts,
                    compile(m, b, ((binary_node_t*)node)->left, e),
                    ((binary_node_t*)node)->right->ts,
                    compile(m, b, ((binary_node_t*)node)->right, e));
            return LLVMBuildXor(b, vpair.left, vpair.right, "multmp");
        }
        case BAND: {
            struct LLVMValueRefPair vpair = ext_int_binaryop_operands(b,
                    ((binary_node_t*)node)->left->ts,
                    compile(m, b, ((binary_node_t*)node)->left, e),
                    ((binary_node_t*)node)->right->ts,
                    compile(m, b, ((binary_node_t*)node)->right, e));
            return LLVMBuildAnd(b, vpair.left, vpair.right, "multmp");
        }

    }

    fprintf(stderr, "ERROR: Undefined eval for operation %d\n!", node->type);
    exit(1);
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
