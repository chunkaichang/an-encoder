
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

#include <iostream>
#include <sstream>

using namespace llvm;

namespace {
  struct AccuPromoter : public FunctionPass {
    AccuPromoter(Coder *c) : FunctionPass(ID), C(c) {}

    bool runOnFunction(Function &F) override;

    static char ID;
  private:
    Coder *C;
  };
}

char AccuPromoter::ID = 0;

bool AccuPromoter::runOnFunction(Function &F) {
  IRBuilder<> builder(&F.getEntryBlock());
  builder.SetInsertPoint(F.getEntryBlock().begin());
    
  for (unsigned i = 0; i < NUM_ACCUS; i++) {

  Value *local_accu = builder.CreateAlloca(C->getInt64Type());
  builder.CreateStore(ConstantInt::getSigned(C->getInt64Type(), 0), local_accu);
  
  std::string accu_name = (i % NUM_ACCUS) ? "accu1_enc" : "accu0_enc";
  GlobalVariable *accu
        = F.getParent()->getNamedGlobal(accu_name);

  for (auto bi = F.begin(), be = F.end(); be != bi; ++bi) {
    for (auto ii = bi->begin(), ie = bi->end(); ii != ie; ++ii) {
      ii->replaceUsesOfWith(accu, local_accu);
    }
  }

  // check:
  for (auto bi = F.begin(), be = F.end(); be != bi; ++bi) {
    assert(!accu->isUsedInBasicBlock(&(*bi)));
  }

  }
  return true;
}

Pass *createAccuPromoter(Coder *c) {
  return new AccuPromoter(c);
}
