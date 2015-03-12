
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Metadata.h"

#include "llvm/Support/raw_ostream.h"

#include <iostream>

using namespace llvm;

namespace {
  struct SExtTruncPass : public BasicBlockPass {
    SExtTruncPass() : BasicBlockPass(ID) {}

    bool runOnBasicBlock(BasicBlock &BB) override;

    static char ID;
  };
}

char SExtTruncPass::ID = 0;

bool SExtTruncPass::runOnBasicBlock(BasicBlock &BB) {
  bool modified = false;
  LLVMContext &ctx = BB.getContext();
  Module *M = BB.getParent()->getParent();

  auto I = BB.begin(), E = BB.end();
  while( I != E) {
    auto N = std::next(I);
    unsigned Op = I->getOpcode();

    switch (Op) {
    case Instruction::SExt: {
      Value *Op = I->getOperand(0);
      if (!isa<TruncInst>(Op))
        break;

      TruncInst *ti = dyn_cast<TruncInst>(Op);
      if (I->getType() == ti->getOperand(0)->getType()) {
        I->replaceAllUsesWith(ti->getOperand(0));
        I->eraseFromParent();
      }
      break;
    }
    case Instruction::Trunc: {
      Value *Op = I->getOperand(0);
      if (!isa<SExtInst>(Op))
        break;

      SExtInst *si = dyn_cast<SExtInst>(Op);
      if (I->getType() == si->getOperand(0)->getType()) {
        I->replaceAllUsesWith(si->getOperand(0));
        I->eraseFromParent();
      }
      break;
    }
    default: {
      break;
    }
    }
    I = N;
  }

  return true;
}

static RegisterPass<SExtTruncPass> X("SExtTruncPass", "Encode initializers of all global variables",
                                     false,
                                     false);

Pass *createSExtTruncPass() {
  return new SExtTruncPass();
}
