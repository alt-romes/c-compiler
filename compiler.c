#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
/* #include <llvm-c/Core.h> */
#include "ast.h"
#include "parse_utils.h"
#include "llvm.h"
#include "dcpuIR.h"


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
            return constant_int(lc, node->num_value);

        case ADD:
            return build_add(b, compile(lc, b, node->children.left), compile(lc, b, node->children.right));

        case SUB:
            return build_sub(b, compile(lc, b, node->children.left), compile(lc, b, node->children.right));

        case MUL:
            return build_mul(b, compile(lc, b, node->children.left), compile(lc, b, node->children.right));

        case DIV:
            return build_div(b, compile(lc, b, node->children.left), compile(lc, b, node->children.right));

        /* case UMINUS: */
        /*     return - eval(node->child, e); */

        default:
            fprintf(stderr, "ERROR: Undefined eval for operation %d\n!", node->type);
            exit(1);
            return NULL;
    }
}

void compile_dcpu(node_t* node) {

    switch (node->type) {

        case NUM:
            emit_num(node->num_value);
            break;

        case ADD:
            compile_dcpu(node->children.left);
            compile_dcpu(node->children.right);
            emit_add();
            break;

        case SUB:
            compile_dcpu(node->children.left);
            compile_dcpu(node->children.right);
            emit_sub();
            break;

        case MUL:
            compile_dcpu(node->children.left);
            compile_dcpu(node->children.right);
            emit_mul();
            break;

        //case DIV:

        case UMINUS:
            compile_dcpu(node->child);
            emit_uminus();
            break;

        default:
            fprintf(stderr, "ERROR: Undefined eval for operation %d\n!", node->type);
            exit(1);
            //return NULL;
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

    compile_dcpu(root);
    dcpu_print();
    dcpu_free();

    LLVMValue* c = compile(lc, builder, root);

    print_llvmvalue(c);

    printf("\n");

    free_ast(root);

    free_llvm((void*)lc);
    free_llvm((void*)mod);
    free_llvm((void*)builder);

    return 0;
}
