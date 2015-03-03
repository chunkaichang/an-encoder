
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
  struct EncodeDecodeRemover : public BasicBlockPass {
    EncodeDecodeRemover() : BasicBlockPass(ID) {}

    bool runOnBasicBlock(BasicBlock &BB) override;

    static char ID;
  };
}

char EncodeDecodeRemover::ID = 0;

bool EncodeDecodeRemover::runOnBasicBlock(BasicBlock &BB) {
  bool modified = false;
  LLVMContext &ctx = BB.getContext();
  Module *M = BB.getParent()->getParent();

  auto I = BB.begin(), E = BB.end();
  while( I != E) {
    auto N = std::next(I);
    unsigned Op = I->getOpcode();

    switch (Op) {
    case Instruction::Call: {
      IntrinsicInst *ii = dyn_cast<IntrinsicInst>(&(*I));
      if (!ii || ii->getIntrinsicID() != Intrinsic::an_decode)
        break;

      Value *Op0 = ii->getArgOperand(0);

      IntrinsicInst *iiarg = dyn_cast<IntrinsicInst>(Op0);
      if (!iiarg || iiarg->getIntrinsicID() != Intrinsic::an_encode)
        break;

      ii->replaceAllUsesWith(iiarg->getArgOperand(0));
      ii->eraseFromParent();
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

static RegisterPass<EncodeDecodeRemover> X("EncodeDecodeRemover", "Encode initializers of all global variables",
                              false,
                              false);

Pass *createEncodeDecodeRemover() {
  return new EncodeDecodeRemover();
}
