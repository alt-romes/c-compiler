#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/Target.h>
#include <llvm-c/Transforms/Scalar.h>
#include <llvm-c/Transforms/Utils.h>
#include <ast.h>
#include <typecheck.h>
#include <parse_utils.h>
#include <debug.h>

#define VERIFY_FUNCTION
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

int eval_constant_expression(node_t* node, int* error) {

    int (*e)(node_t*, int*) = eval_constant_expression;

    int a = 0, b = 0;
    switch (node->type) {
        /* Binary nodes that need same-size type operands in common */
        case ADD: case SUB: case MUL: case DIV: case REM:
        case EQ: case NE: case LT: case GT: case LE: case GE:
        case LOR: case LAND: case BAND: case BOR: case BXOR:
        case LEFT_SHIFT: case RIGHT_SHIFT:
            a = e(((binary_node_t*)node)->left, error);
            b = e(((binary_node_t*)node)->right, error);
            break;
        case UPLUS: case UMINUS:
            a = e(((unary_node_t*)node)->child, error);
            break;
        default:
            break;
    }

    switch (node->type) {
        case NUM: return ((num_node_t*)node)->value;
        case ADD: return a + b;
        case SUB: return a - b;
        case MUL: return a * b;
        case DIV: return a / b;
        case REM: return a % b;
        case EQ: return a == b;
        case NE: return a != b;
        case LT: return a < b;
        case GT: return a > b;
        case LE: return a <= b;
        case GE: return a >= b;
        case LOR: return a || b;
        case LAND: return a && b;
        case BAND: return a & b;
        case BOR: return a | b;
        case BXOR: return a ^ b;
        case LEFT_SHIFT: return a << b;
        case RIGHT_SHIFT: return a >> b;
        case UPLUS: return a;
        case UMINUS: return -a;
        case LOGICAL_NOT: return !a;
        case BNOT: return ~a;
        case CONDITIONAL: return e(((if_node_t*)node)->cond, error) ? e(((if_node_t*)node)->thenst, error) : e(((if_node_t*)node)->elsest, error);
        default:
            // Not a constant expression, so return zero which will be handled
            *error = 1;
            return 1;
    }

}

LLVMTypeRef type2LLVMType(type_t ts) {

    LLVMTypeRef tr = NULL;

    switch (ts->t & 0xFFFF) {
        case VOID:
            tr = LLVMVoidType();
            break;
        case I1:
            tr = LLVMInt1Type();
            break;
        case CHAR:
            tr = LLVMInt8Type();
            break;
        case SHORT:
            tr = LLVMInt16Type();
            break;
        case INT:
            tr = LLVMInt32Type();
            break;
        case LONG:
            tr = LLVMInt64Type();
            break;
        case POINTER:
            tr = LLVMPointerType(type2LLVMType(((pointer_type_t)ts)->pointed), 0 /* TODO: LLVM ADDRESS SPACE */);
            break;
        case FUNCTION_TYPE: {

            function_type_t fts = (function_type_t)ts;
            LLVMTypeRef ret_type = type2LLVMType(fts->ret);

            // Create function with parameters when it has been defined with them vvvv
            int argc = fts->args->size;
            LLVMTypeRef* params = malloc(sizeof(LLVMTypeRef)*argc);

            // Define function type with correct parameters
            for (int i = 0; i < argc; i++)
                params[i] = type2LLVMType(fts->args->args[i].ts);

            tr = LLVMFunctionType(ret_type, params, argc, 0);

            free(params); // No longer needed;

            break;
        }
        case ARRAY_TYPE: {

            // No length arrays are zero length arrays.
            // Try to eval the constant expression.
            // If a non constant element is found, the array size isn'ti known at compile time,
            // and is actually a variable length array: https://gcc.gnu.org/onlinedocs/gcc/Variable-Length.html
            
            int size = 0, error = 0;
            if (((array_type_t)ts)->size == NULL ||
                    ((size = eval_constant_expression(((array_type_t)ts)->size, &error)) && error == 0))
                // Array has no size or has a constant size that was evaluated
                tr = LLVMArrayType(type2LLVMType(((array_type_t)ts)->elems), size);
            else {
                // Array is a variable length array
                fprintf(stderr, "Variable length arrays are not yet supported\n");
                exit(83);
                tr = NULL;
            }
            break;
        }
        default:
            fprintf(stderr, "type2LLVM undefined for ts: %d\n", ts->t);
            exit(1);
    }

    return tr;

}

struct LLVMValueRefPair {
    LLVMValueRef left, right;
};

struct LLVMValueRefPair ext_int_binaryop_operands(LLVMBuilderRef b, type_t lnt, LLVMValueRef left, type_t rnt, LLVMValueRef right) {

    LLVMTypeRef lt = LLVMTypeOf(left);
    LLVMTypeRef rt = LLVMTypeOf(right);

    if (type_compare(lnt, rnt) < 0) // left is smaller than right
        left = (is_type_unsigned(lnt) ? LLVMBuildZExt : LLVMBuildSExt)(b, left, rt, "sextlefttmp"); // Extend left type to match right type size
    else if (type_compare(lnt, rnt) > 0) // left is larger than right
        right = (is_type_unsigned(rnt) ? LLVMBuildZExt : LLVMBuildSExt)(b, right, lt, "sextrighttmp"); // Extend right type to match left type size

    return (struct LLVMValueRefPair){ left, right };
}

LLVMValueRef cast(LLVMBuilderRef b, type_t dst_type, type_t src_type, LLVMValueRef src) {

    if (type_compare(src_type, dst_type) < 0)
        src = (is_type_unsigned(src_type) ? LLVMBuildZExt : LLVMBuildSExt)(b, src, type2LLVMType(dst_type), "extsrctmp");
    else if (type_compare(src_type, dst_type) > 0)
        src = LLVMBuildTrunc(b, src, type2LLVMType(dst_type), "truncsrctmp"); // extend right type to match left type size

    // TODO: Cast int to float, vice-versa, etc...

    return src;
}

/* 
 * Transform an integer number into a boolean as an llvm i1 register value
 * @b -> Instruction builder
 * @a -> Value to convert to boolean, value == 0 => false (i1=0) ^ value != 1 => true (i1=1)
 * @a_type -> Int type size of a value
 */
LLVMValueRef llvmInt2BoolI1(LLVMBuilderRef b, LLVMValueRef a, type_t a_type) {
    return LLVMBuildNot(b, LLVMBuildICmp(b, LLVMIntEQ, a, LLVMConstInt(type2LLVMType(a_type), 0, 0), "tobooli1tmp"), "tobooli1nottmp");
}


/*
 * The last parameter is called "current_function_type_or_auto_deref_stack", or "cftoads".
 * When current_function_type_or_auto_deref_stack == -1:
 *      => compiling an ID will be just getting the memory location associated with it
 * When current_function_type_or_auto_deref_stack != -1 (enum type)
 *      => when compiling an ID, its value is loaded from the associated memory location,
 *      => return nodes are automatically cast to the current function type encoded directly in the argument (this will probably have to be split into a different argument :( because of REFOF)
 * There is no conflict when we assume that a return node will never be present in a NO_AUTO_DEREF situation,
 * so the only time the parameter is -1, the possibly encoded type isn't needed because no return statement will be cast
 */
#define NO_AUTO_DEREF type_from(UNDEFINED)
LLVMValueRef compile(LLVMModuleRef m, LLVMBuilderRef b, node_t* node,
        environment_t* e, type_t cftoads) {

    union {
        LLVMIntPredicate llvmIntPredicate;
        struct LLVMValueRefPair llvmVPair;
    } aux;

    aux.llvmIntPredicate = 0;

    switch (node->type) {
        /* Binary nodes that need same-size type operands in common */
        case ADD: case SUB: case MUL: case DIV: case REM:
        case BAND: case BOR: case BXOR:
        case LEFT_SHIFT: case RIGHT_SHIFT:
            aux.llvmVPair = ext_int_binaryop_operands(b,
                ((binary_node_t*)node)->left->ts,
                compile(m, b, ((binary_node_t*)node)->left, e, cftoads),
                ((binary_node_t*)node)->right->ts,
                compile(m, b, ((binary_node_t*)node)->right, e, cftoads));
            break;
        /* Binary nodes that use different llvmIntPredicates */
        case EQ: case NE: case LT: case GT: case LE: case GE:
            aux.llvmIntPredicate = 0;
            break;
        default:
            break;
    }

    switch (node->type) {

        case FUNCTION: {
            function_node_t* fnode = (function_node_t*)node;

            LLVMTypeRef ret_type = type2LLVMType(((function_type_t)node->ts)->ret);

            // Create function with parameters when it has been defined with them vvvv
            struct args_list* args_list = ((function_type_t)fnode->decl.ts)->args;
            int argc = args_list->size;

            // Define function type with correct parameters
            LLVMTypeRef* params = malloc(sizeof(LLVMTypeRef)*argc);
            for (int i = 0; i < argc; i++)
                params[i] = type2LLVMType(args_list->args[i].ts);

            LLVMTypeRef fun_type = LLVMFunctionType(ret_type, params, argc, 0);

            LLVMValueRef fun = LLVMAddFunction(m, ((function_node_t*)node)->decl.id, fun_type);
            LLVMBasicBlockRef entry = LLVMAppendBasicBlock(fun, "entry");
            LLVMPositionBuilderAtEnd(b, entry);

            environment_t* scope_env = beginScope(e);

            // Add params to environment
            for (int i = 0; i < argc; i++) {
                LLVMValueRef param_alloca = LLVMBuildAlloca(b, params[i], "allocatmp");
                LLVMBuildStore(b, LLVMGetParam(fun, i), param_alloca);
                assoc(scope_env, args_list->args[i].id, (union association_v){ .llvmref = param_alloca });
            }

            free(params); // No longer needed;

            compile(m, b, ((function_node_t*)node)->body, scope_env, ((function_type_t)node->ts)->ret /* update current function return type */);

            endScope(scope_env); // free the scope environment and its association array

#ifdef VERIFY_FUNCTION
            LLVMVerifyFunction(fun, LLVMAbortProcessAction);
#endif
#ifdef OPTIMIZE
            // Run optimizations!
            LLVMRunFunctionPassManager(pass_manager, fun);
#endif
            return fun;
        }

        case ID: {
            debugf1("Compile ID %s", ((id_node_t*)node)->value);
            LLVMValueRef alloca = find(e, ((id_node_t*)node)->value).llvmref;
            debugf1("Found ID %s", ((id_node_t*)node)->value);
            if (cftoads->t != NO_AUTO_DEREF->t) {
                debugf1("Variable: %s", LLVMPrintValueToString(alloca));
                debugf1("Variable type: %s", LLVMPrintTypeToString(type2LLVMType(node->ts)));
                debug("Load ID");
                return LLVMBuildLoad2(b, type2LLVMType(node->ts), alloca, "loadtmp");
            }
            else {
                debug("ID without load");
                return alloca;
            }
        }

        case GLOBAL_BLOCK:
        case BLOCK: {

            environment_t* scope_env = beginScope(e);

            debug("Block begin scope");
            
            block_node_t* bnode = (block_node_t*)node;
            declaration_list_t* dae  = bnode->declaration_list;  // this id->ast_node environment is freed when the whole ast is freed
                                                                // NOTE: the ids will be freed with the ast (they were allocated with it, so they should be deallocated with it)

            debug_type("Block type", node->ts);
            if (node->type == GLOBAL_BLOCK) { // Is this the global "pseudo" block

                debug("Block is global");
                for (int i = 0; i < dae->size; i++) {

                    // Function nodes are added as functions to the module
                    if (dae->declarations[i].et->t & FUNCTION_TYPE)
                        if (dae->declarations[i].node != NULL)
                            // Function with body
                            assoc(scope_env, dae->declarations[i].id, (union association_v){ .llvmref = compile(m, b, dae->declarations[i].node, scope_env, cftoads) });
                        else {
                            // Function without body
                            // Declare prototype (TODO: this should be factored out somewhere)

                            LLVMTypeRef ret_type = type2LLVMType(((function_type_t)dae->declarations[i].et)->ret);

                            struct args_list* args_list = ((function_type_t)dae->declarations[i].et)->args;
                            int argc = args_list->size;
                            LLVMTypeRef* params = malloc(sizeof(LLVMTypeRef)*argc);
                            for (int i = 0; i < argc; i++)
                                params[i] = type2LLVMType(args_list->args[i].ts);

                            LLVMTypeRef fun_type = LLVMFunctionType(ret_type, params, argc, 0);
                            free(params);

                            LLVMValueRef fun = LLVMAddFunction(m, dae->declarations[i].id, fun_type);
                            LLVMSetLinkage(fun, LLVMExternalLinkage);
                            assoc(scope_env, dae->declarations[i].id, (union association_v){ .llvmref = fun });
                        }
                    else {

                        LLVMValueRef alloca = LLVMAddGlobal(m, type2LLVMType(dae->declarations[i].et), dae->declarations[i].id);
                        LLVMValueRef assignment_val = LLVMConstInt(type2LLVMType(dae->declarations[i].et), 0, 1);
                        if (dae->declarations[i].node != NULL)
                            assignment_val = cast(b, dae->declarations[i].et, dae->declarations[i].node->ts, compile(m, b, dae->declarations[i].node, scope_env, cftoads));
                        LLVMSetInitializer(alloca, assignment_val);
                        if (dae->declarations[i].et->t & CONST)
                            LLVMSetGlobalConstant(alloca, 1);
                        assoc(scope_env, dae->declarations[i].id, (union association_v){ .llvmref = alloca });
                    }
                }

            }
            else {

                debug("Block is NOT global");

                // For each declaration in this scope create an association in this scope's evaluation environment
                for (int i = 0; i < dae->size; i++) {
                    LLVMValueRef alloca = LLVMBuildAlloca(b, type2LLVMType(dae->declarations[i].et), "allocatmp");
                    LLVMValueRef assignment_val = NULL;
                    if (dae->declarations[i].node != NULL) {
                        assignment_val = compile(m, b, dae->declarations[i].node, scope_env, cftoads);
                        LLVMBuildStore(b, cast(b, dae->declarations[i].et, dae->declarations[i].node->ts, assignment_val) /* cast value to id type */, alloca);
                    }
                    assoc(scope_env, dae->declarations[i].id, (union association_v){ .llvmref = alloca });
                }

                statement_list_t* statement_list = ((block_node_t*)node)->statement_list;
                for (int i = 0; i < statement_list->size; i++)
                    compile(m, b, statement_list->statements[i], scope_env, cftoads);

            }

            endScope(scope_env); // free the scope environment and its association array

            return NULL;
        }

        case CALL: {
            debug("Compiling call");
            statement_list_t* args_list = (statement_list_t*)((binary_node_t*)node)->right;
            function_node_t* ast_func = (function_node_t*)((binary_node_t*)node)->left;
            debug("Compiling id to get function");
            LLVMValueRef func = compile(m, b, (node_t*)ast_func, e, NO_AUTO_DEREF);
            // TODO: Read related TODO in typecheck CALL
            function_type_t func_type = ast_func->ts->t & POINTER ? (function_type_t)((pointer_type_t)ast_func->ts)->pointed : (function_type_t)ast_func->ts;
            if (ast_func->ts->t & POINTER) {
                debug("Auto dereferencing function pointer");
                debugf1("Function type %s", LLVMPrintTypeToString(type2LLVMType(ast_func->ts)));
                func = LLVMBuildLoad2(b, type2LLVMType(ast_func->ts), func, "loadfp");
            }
            debugf1("Got function pseudo alloca %s", ast_func->decl.id);
            LLVMValueRef* args_values = malloc(args_list->size*sizeof(LLVMValueRef));
            for (int i = 0; i < args_list->size; i++) {
                debugf1("Compile call argument %s", func_type->args->args[i].id);
                args_values[i] = compile(m, b, args_list->statements[i], e, cftoads);
                debug("Casting call argument to expected function type");
                args_values[i] = cast(b, func_type->args->args[i].ts, args_list->statements[i]->ts, args_values[i]);
            }
            debug("All call arguments compiled");
            debug("Building llvm call type");
            debug_type("llvm call type", (type_t)func_type);
            LLVMTypeRef callType = type2LLVMType((type_t)func_type);
            debug("Building llvm call value");
            LLVMValueRef callValue = LLVMBuildCall2(b, callType, func, args_values, args_list->size, "");
            debug("Call value built!");
            return callValue;
        }

        // All XXX_ASSIGNS have been desugered to an ASSIGN,
        // but maintain their type to be correctly freed.
        // Treat them equally.
        case ASSIGN:
        case MUL_ASSIGN:
        case DIV_ASSIGN:
        case MOD_ASSIGN:
        case ADD_ASSIGN:
        case SUB_ASSIGN:
        case LEFT_ASSIGN:
        case RIGHT_ASSIGN:
        case AND_ASSIGN:
        case XOR_ASSIGN:
        case OR_ASSIGN: {
            LLVMValueRef lhs = compile(m, b, ((binary_node_t*)node)->left, e, NO_AUTO_DEREF);
            LLVMValueRef rhs = compile(m, b, ((binary_node_t*)node)->right, e, cftoads);
            LLVMValueRef rhsX = cast(b, ((binary_node_t*)node)->left->ts, ((binary_node_t*)node)->right->ts, rhs);
            LLVMBuildStore(b, rhsX, lhs);
            return rhs;
        }

        case SEQEXP:
            return compile(m, b, ((binary_node_t*)node)->left, e, cftoads), compile(m, b, ((binary_node_t*)node)->right, e, cftoads);

        case NUM:
            return LLVMConstInt(type2LLVMType(node->ts), ((num_node_t*)node)->value, is_type_unsigned(node->ts) ? 0 : 1);

        case ADD:
            return LLVMBuildAdd(b, aux.llvmVPair.left, aux.llvmVPair.right, "addtmp");

        case SUB:
            return LLVMBuildSub(b, aux.llvmVPair.left, aux.llvmVPair.right, "subtmp");

        case MUL:
            return LLVMBuildMul(b, aux.llvmVPair.left, aux.llvmVPair.right, "multmp");

        case DIV:
            return (is_type_unsigned(node->ts)?LLVMBuildUDiv:LLVMBuildSDiv)(b, aux.llvmVPair.left, aux.llvmVPair.right, "divtmp");

        case REM:
            return (is_type_unsigned(node->ts)?LLVMBuildURem:LLVMBuildSRem)(b, aux.llvmVPair.left, aux.llvmVPair.right, "remtmp");

        // LT, GT, LE, and GE will have type unsigned if the comparison is between unsigned types, and signed type otherwise
        case LT:
            aux.llvmIntPredicate = aux.llvmIntPredicate == 0 ? is_type_unsigned(node->ts) ? LLVMIntULT : LLVMIntSLT : aux.llvmIntPredicate; 
        case GT:
            aux.llvmIntPredicate = aux.llvmIntPredicate == 0 ? is_type_unsigned(node->ts) ? LLVMIntUGT : LLVMIntSGT : aux.llvmIntPredicate; 
        case LE:
            aux.llvmIntPredicate = aux.llvmIntPredicate == 0 ? is_type_unsigned(node->ts) ? LLVMIntULE : LLVMIntSLE : aux.llvmIntPredicate; 
        case GE:
            aux.llvmIntPredicate = aux.llvmIntPredicate == 0 ? is_type_unsigned(node->ts) ? LLVMIntUGE : LLVMIntSGE : aux.llvmIntPredicate; 
        case NE:
            aux.llvmIntPredicate = aux.llvmIntPredicate == 0 ? LLVMIntNE : aux.llvmIntPredicate; 
        case EQ: {
            aux.llvmIntPredicate = aux.llvmIntPredicate == 0 ? LLVMIntEQ : aux.llvmIntPredicate; 

            struct LLVMValueRefPair vpair = ext_int_binaryop_operands(b,
                    ((binary_node_t*)node)->left->ts,
                    compile(m, b, ((binary_node_t*)node)->left, e, cftoads),
                    ((binary_node_t*)node)->right->ts,
                    compile(m, b, ((binary_node_t*)node)->right, e, cftoads));
            // Return boolean extended to relational operation type size (CHAR) based on auxiliary LLVMIntPredicate
            return LLVMBuildICmp(b, aux.llvmIntPredicate, vpair.left, vpair.right , "cmptmp");
        }

        case PRE_INC:
        case PRE_DEC:
        case POST_INC:
        case POST_DEC:
            return compile(m, b, ((unary_node_t*)node)->child, e, cftoads);

        // Booleans are represented in an i1 LLVM register, we can convert a number to an i1 boolean, and bit OR, AND, NOT operations on 1 bit are the same as logical operations
        case LOR:
            return LLVMBuildOr(b,
                    llvmInt2BoolI1(b,
                        compile(m, b, ((binary_node_t*)node)->left, e, cftoads),
                        ((binary_node_t*)node)->left->ts),
                    llvmInt2BoolI1(b,
                        compile(m, b, ((binary_node_t*)node)->right, e, cftoads),
                        ((binary_node_t*)node)->right->ts),
                    "ortmp");
        case LAND:
            return LLVMBuildAnd(b,
                    llvmInt2BoolI1(b,
                        compile(m, b, ((binary_node_t*)node)->left, e, cftoads),
                        ((binary_node_t*)node)->left->ts),
                    llvmInt2BoolI1(b,
                        compile(m, b, ((binary_node_t*)node)->right, e, cftoads),
                        ((binary_node_t*)node)->right->ts),
                    "andtmp");

        case DEREF: {
            /* debug_type("Loading", node->ts); */
            LLVMValueRef alloca = compile(m, b, ((unary_node_t*)node)->child, e, cftoads);
            if (cftoads->t != NO_AUTO_DEREF->t)
                // Dereference on the right side -> get final value
                return LLVMBuildLoad2(b, type2LLVMType(node->ts), alloca, "loadrhstmp");
            else
                // Dereference on the left side -> get reference of final value (because the Id won't be dereferenced, this is actually transparent, we just need to make sure we load the correct types)
                return LLVMBuildLoad2(b, LLVMGetElementType(LLVMTypeOf(alloca)), alloca, "loadlhstmp");
                
        }
        case REFOF:
            return compile(m, b, ((unary_node_t*)node)->child, e, NO_AUTO_DEREF); // Compile without auto deref :)

        case UPLUS:
            return compile(m, b, ((unary_node_t*)node)->child, e, cftoads);
        
        case UMINUS:
            return LLVMBuildNeg(b, compile(m, b, ((unary_node_t*)node)->child, e, cftoads), "negtmp");

        case LOGICAL_NOT:
            // Possibly innefficient converting numbers to i1 booleans before doing boolean operations
            return LLVMBuildNot(b, llvmInt2BoolI1(b, compile(m, b, ((unary_node_t*)node)->child, e, cftoads), ((unary_node_t*)node)->child->ts), "logicalnottmp");

        case BNOT:
            return LLVMBuildNot(b, compile(m, b, ((unary_node_t*)node)->child, e, cftoads), "bnottmp");

        case BOR:
            return LLVMBuildOr(b,  aux.llvmVPair.left, aux.llvmVPair.right, "bortmp");
        case BXOR:
            return LLVMBuildXor(b, aux.llvmVPair.left, aux.llvmVPair.right, "xortmp");
        case BAND:
            return LLVMBuildAnd(b, aux.llvmVPair.left, aux.llvmVPair.right, "bandtmp");
        case LEFT_SHIFT:
            return LLVMBuildShl(b, aux.llvmVPair.left, aux.llvmVPair.right, "leftshifttmp");
        case RIGHT_SHIFT:
            return (is_type_unsigned(node->ts)?LLVMBuildLShr:LLVMBuildAShr)(b, aux.llvmVPair.left, aux.llvmVPair.right, "rightshifttmp");

        case RETURN:
            if (((unary_node_t*)node)->child != NULL)
                return LLVMBuildRet(b, cast(b, cftoads, ((unary_node_t*)node)->child->ts, compile(m, b, ((unary_node_t*)node)->child, e, cftoads)));
            else
                return LLVMBuildRetVoid(b);
        case CAST:
            return cast(b, node->ts, ((unary_node_t*)node)->child->ts, compile(m, b, ((unary_node_t*)node)->child, e, cftoads));

        case IF:
        case CONDITIONAL: {
            if_node_t* ifnode = (if_node_t*)node;
            LLVMValueRef condition = llvmInt2BoolI1(b, compile(m, b, ifnode->cond, e, cftoads), ifnode->cond->ts);

            // Retrieve function.
            LLVMValueRef func = LLVMGetBasicBlockParent(LLVMGetInsertBlock(b));

            // Generate true/false expr and merge.
            LLVMBasicBlockRef then_block = LLVMAppendBasicBlock(func, "then");
            LLVMBasicBlockRef else_block = LLVMAppendBasicBlock(func, "else");
            LLVMBasicBlockRef merge_block = LLVMAppendBasicBlock(func, "ifcont");

            LLVMBuildCondBr(b, condition, then_block, else_block);

            // Generate 'then' block.
            LLVMPositionBuilderAtEnd(b, then_block);
            LLVMValueRef then_value = compile(m, b, ((if_node_t*)node)->thenst, e, cftoads);
            LLVMBuildBr(b, merge_block);

            LLVMPositionBuilderAtEnd(b, else_block);
            LLVMValueRef else_value;
            if (ifnode->elsest != NULL) {
                else_value = compile(m, b, ifnode->elsest, e, cftoads);
                // If the node is a conditional, the else branch must be cast to the same type as the then branch
                if (node->type == CONDITIONAL)
                    else_value = cast(b, ifnode->thenst->ts, ifnode->elsest->ts, else_value);
            }
            LLVMBuildBr(b, merge_block);

            LLVMPositionBuilderAtEnd(b, merge_block);
            if (node->type == IF)
                return NULL;
            else if (node->type == CONDITIONAL) {

                LLVMPositionBuilderAtEnd(b, merge_block);
                LLVMValueRef phi = LLVMBuildPhi(b, type2LLVMType(node->ts), "phitmp");
                LLVMAddIncoming(phi, &then_value, &then_block, 1);
                LLVMAddIncoming(phi, &else_value, &else_block, 1);
                
                return phi;
            }

        }
        case UNIT:
            // Do nothing
            return NULL;
    }

    fprintf(stderr, "ERROR: Undefined compilation for operation %d\n!", node->type);
    exit(1);
}

int main(int argc, char *argv[]) {

    debug("\n\n\n[ Parsing ]");
    node_t* root = parse_root();

    debug("\n\n\n[ Type Checking ]");
    environment_t* typing_env = newEnvironment();
    typecheck(root, typing_env);
    free(typing_env);

    debug("\n\n\n[ Setup ]");
    LLVMModuleRef module = LLVMModuleCreateWithName("llvm!"); // new, empty module in the global context
    LLVMBuilderRef builder = LLVMCreateBuilder();          // builder in the global context starting at entry

#ifdef OPTIMIZE
    create_llvm_pass_manager(module); // Initialize global pass_manager
#endif

    environment_t* env = newEnvironment();

    debug("\n\n\n[ Compiling ]");
    compile(module, builder, root, env, type_from(VOID));

    debug("\n\n\n[ Cleaning ]");
    free(env);
    free_ast(root);
    free_all_types();

    debug("\n\n\n[ Printing ]");
    char* module_string = LLVMPrintModuleToString(module);
    debug("\n\n\n[ Module ]");
    printf("%s", module_string);

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
