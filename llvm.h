#ifndef _LLVM_H
#define _LLVM_H

#include "ast.h"

#ifndef __cplusplus

typedef struct LLVMContext LLVMContext;
typedef struct Module Module;
typedef struct IRBuilder IRBuilder;
typedef struct LLVMValue LLVMValue;

#else

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
using namespace llvm;

#define LLVMValue Value
#define IRBuilder IRBuilder<>

#endif


#ifdef __cplusplus
// Functions callable from C
extern "C" {
#endif

LLVMContext* initialize_context();
Module* initialize_module(char*, LLVMContext*);
IRBuilder* initialize_builder(LLVMContext*);

LLVMValue* codegen(node_t*, IRBuilder*);

void print_module_llvm(Module*);
void print_llvmvalue(LLVMValue*);

void free_llvm(void*);

LLVMValue* constant_int(LLVMContext*, int);
LLVMValue* build_add(IRBuilder*, LLVMValue*, LLVMValue*);
LLVMValue* build_sub(IRBuilder*, LLVMValue*, LLVMValue*);
LLVMValue* build_mul(IRBuilder*, LLVMValue*, LLVMValue*);
LLVMValue* build_div(IRBuilder*, LLVMValue*, LLVMValue*);

#ifdef __cplusplus
}
#endif

#endif /* _LLVM_H */
