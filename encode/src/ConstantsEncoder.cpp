
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

#include "coder/ProfiledCoder.h"

using namespace llvm;

namespace {
  struct ConstantsEncoder : public BasicBlockPass {
    ConstantsEncoder(ProfiledCoder *pc) : BasicBlockPass(ID), PC(pc) {}

    bool runOnBasicBlock(BasicBlock &BB) override;

    static char ID;
  private:
    ProfiledCoder *PC;
  };
}

char ConstantsEncoder::ID = 0;

bool ConstantsEncoder::runOnBasicBlock(BasicBlock &BB) {
  if (!BB.getParent()->getName().compare("main"))
    return false;

  bool modified = false;

  for (auto I = BB.begin(), E = BB.end(); I != E; I++) {
    if (isa<AllocaInst>(&(*I)))
      continue;

    for (unsigned i = 0; i < I->getNumOperands(); i++) {
      Value *Op = I->getOperand(i);
      ConstantInt *CI = dyn_cast<ConstantInt>(Op);
      if (!CI)
        continue;
      IntegerType *type = dyn_cast<IntegerType>(Op->getType());
      if (type->getBitWidth() != 64)
    	  continue;

      int64_t value = CI->getSExtValue();
      int64_t res = value * PC->getA();
      ConstantInt *CodedCI = ConstantInt::getSigned(dyn_cast<IntegerType>(PC->getInt64Type()),
                                                    res);
      I->setOperand(i, CodedCI);

      modified = true;
    }
  }

  return modified;
}

Pass *createConstantsEncoder(ProfiledCoder *pc) {
  return new ConstantsEncoder(pc);
}
