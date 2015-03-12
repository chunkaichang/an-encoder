
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Metadata.h"

#include "Coder.h"

using namespace llvm;

namespace {
  struct GEPHandler : public BasicBlockPass {
    GEPHandler(Coder *c) : BasicBlockPass(ID), C(c) {}

    bool runOnBasicBlock(BasicBlock &BB) override;

    static char ID;
  private:
    Coder *C;
  };
}

char GEPHandler::ID = 0;

bool GEPHandler::runOnBasicBlock(BasicBlock &BB) {
  bool modified = false;
  Module *M = BB.getParent()->getParent();

  for (auto I = BB.begin(), E = BB.end(); I != E; I++) {
    GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(&(*I));

    if (GEP) {
      for (unsigned i = 0; i < I->getNumOperands(); i++) {
        Value *Op = I->getOperand(i);
        if (!Op->getType()->isIntegerTy())
          continue;

        Value *NewOp = C->createEncRegionExit(Op, Op->getType(), I);
        I->setOperand(i, NewOp);

        modified = true;
      }
    }
  }

  return modified;
}

Pass *createGEPHandler(Coder *c) {
  return new GEPHandler(c);
}
