
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Metadata.h"

#include "llvm/Support/raw_ostream.h"

#include <iostream>

using namespace llvm;

namespace {
  struct AccumulateRemover : public BasicBlockPass {
    AccumulateRemover() : BasicBlockPass(ID) {}

    bool runOnBasicBlock(BasicBlock &BB) override;

    static char ID;
  };
}

char AccumulateRemover::ID = 0;

bool AccumulateRemover::runOnBasicBlock(BasicBlock &BB) {
  bool modified = false;
  LLVMContext &ctx = BB.getContext();
  Module *M = BB.getParent()->getParent();

  auto I = BB.begin(), E = BB.end();
  while( I != E) {
    auto N = std::next(I);
    CallInst *ci = dyn_cast<CallInst>(I);
    Function *callee = ci ? ci->getCalledFunction() : nullptr;
    if (callee && callee->getName().equals("accumulate_enc")) {
        I->eraseFromParent();
        modified = true;
    }
    I = N;
  }

  return modified;
}

static RegisterPass<AccumulateRemover> X("AccumulateRemover",
                                         "",
                                         false,
                                         false);

Pass *createAccumulateRemover() {
  return new AccumulateRemover();
}
