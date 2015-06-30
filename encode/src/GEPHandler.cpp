
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"

#include "Coder.h"
#include "UsesVault.h"

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
      Value *Ptr = GEP->getPointerOperand();
      if (!dyn_cast<GlobalValue>(Ptr->stripPointerCasts())) {
      	//C->createAssert(Ptr, &(*I));
        GEP->setOperand(0, C->createDecode(Ptr, GEP));
      }

      for (unsigned i = 1; i < I->getNumOperands(); i++) {
        Value *Op = I->getOperand(i);
        if (!Op->getType()->isIntegerTy())
          continue;

        Value *NewOp = C->createEncRegionExit(Op, Op->getType(), I);
        NewOp = C->createTrunc(NewOp, C->getInt32Type(), I); 
        I->setOperand(i, NewOp);

        modified = true;
      }

      // Since pointers are assumed to be encoded, we must encode
      // the result of a 'GEP' instruction:
      {
        UsesVault UV(I->uses());
        Value *enc = C->createEncode(I, std::next(I));
        UV.replaceWith(enc);
      }
    }
  }

  return modified;
}

Pass *createGEPHandler(Coder *c) {
  return new GEPHandler(c);
}
