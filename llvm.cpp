#include <iostream>
#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include "llvm.h"

#undef IRBuilder
using namespace std;

LLVMContext* initialize_context() {
    /* auto p = (unique_ptr<LLVMContext>*)malloc(sizeof(unique_ptr<LLVMContext>)); */
    /* *p = make_unique<LLVMContext>(); */
    /* return p->get(); */

    /* LLVMContext* p = (LLVMContext*)malloc(sizeof(LLVMContext)); */
    return new LLVMContext();
}

Module* initialize_module(char* str, LLVMContext* lc) {
    return new Module(str, *lc);
}

IRBuilder<>* initialize_builder(LLVMContext* lc) {
    return new IRBuilder<>(*lc);
}

void print_module_llvm(Module* m) {
    m->print(errs(), nullptr);
}

void print_value_llvm(LLVMValue* v) {
    v->print(errs());
}

void print_int(LLVMContext* lc, int x) {
    ConstantInt::get(*lc, APInt(32, x))->print(errs()); 
    cout << "\n";
}
