#include <iostream>
#include <llvm/ADT/APInt.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/PassPlugin.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Analysis/AliasAnalysis.h>
#include "llvm.h"

#undef IRBuilder
using namespace std;
using namespace llvm;

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

void llvm_optimize(LLVMModuleRef m) {

    // Create the analysis managers.
    LoopAnalysisManager LAM;
    FunctionAnalysisManager FAM;
    CGSCCAnalysisManager CGAM;
    ModuleAnalysisManager MAM;

    // Create the new pass manager builder.
    // Take a look at the PassBuilder constructor parameters for more
    // customization, e.g. specifying a TargetMachine or various debugging
    // options.
    PassBuilder PB;

    // Make sure to use the default alias analysis pipeline, otherwise we'll end
    // up only using a subset of the available analyses.
    FAM.registerPass([&] { return PB.buildDefaultAAPipeline(); });

    // Register all the basic analyses with the managers.
    PB.registerModuleAnalyses(MAM);
    PB.registerCGSCCAnalyses(CGAM);
    PB.registerFunctionAnalyses(FAM);
    PB.registerLoopAnalyses(LAM);
    PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

    // Create the pass manager.
    // This one corresponds to a typical -O2 optimization pipeline.
    ModulePassManager MPM = PB.buildPerModuleDefaultPipeline(PassBuilder::OptimizationLevel::O2);

    // Optimize the IR!
    MPM.run(*unwrap(m), MAM);

}
