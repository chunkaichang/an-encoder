
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Metadata.h"

#include "coder/ProfiledCoder.h"

#include <sstream>

using namespace llvm;

namespace {
  struct AccuPromoter : public FunctionPass {
    AccuPromoter(ProfiledCoder *pc) : FunctionPass(ID), PC(pc) {}

    bool runOnFunction(Function &F) override;

    static char ID;
  private:
    ProfiledCoder *PC;
  };
}

char AccuPromoter::ID = 0;

bool AccuPromoter::runOnFunction(Function &F) {
  IRBuilder<> builder(&F.getEntryBlock());
  builder.SetInsertPoint(F.getEntryBlock().begin());
    
  Value *local_accu = builder.CreateAlloca(PC->getInt64Type());
  builder.CreateStore(ConstantInt::getSigned(PC->getInt64Type(), 0), local_accu);
  
  GlobalVariable *accu = F.getParent()->getNamedGlobal("accu_enc");

  for (auto bi = F.begin(), be = F.end(); be != bi; ++bi) {
    for (auto ii = bi->begin(), ie = bi->end(); ii != ie; ++ii) {
      ii->replaceUsesOfWith(accu, local_accu);
    }
  }

  // check:
  for (auto bi = F.begin(), be = F.end(); be != bi; ++bi) {
    assert(!accu->isUsedInBasicBlock(&(*bi)));
  }
  return true;
}

Pass *createAccuPromoter(ProfiledCoder *pc) {
  return new AccuPromoter(pc);
}
