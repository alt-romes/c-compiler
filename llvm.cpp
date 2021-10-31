#include <iostream>
#include <llvm/ADT/APInt.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include "llvm.h"

#undef IRBuilder
using namespace std;

LLVMContext* initialize_context() { return new LLVMContext(); }

Module* initialize_module(char* str, LLVMContext* lc) { return new Module(str, *lc); }

IRBuilder<>* initialize_builder(LLVMContext* lc) { return new IRBuilder<>(*lc); }

void print_module_llvm(Module* m) { m->print(errs(), nullptr); }

void print_llvmvalue(LLVMValue* v) { v->print(errs()); }

void free_llvm(void* p) { free(p); }

LLVMValue* constant_int(LLVMContext* lc, int x) { return ConstantInt::get(*lc, APInt(32, x)); }

LLVMValue* build_add(IRBuilder<>* b, LLVMValue* l, LLVMValue* r) { return b->CreateAdd(l, r); }
LLVMValue* build_sub(IRBuilder<>* b, LLVMValue* l, LLVMValue* r) { return b->CreateSub(l, r); }
LLVMValue* build_mul(IRBuilder<>* b, LLVMValue* l, LLVMValue* r) { return b->CreateMul(l, r); }
LLVMValue* build_div(IRBuilder<>* b, LLVMValue* l, LLVMValue* r) { return b->CreateSDiv(l, r); }

