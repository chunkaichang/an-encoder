
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

#include "Coder.h"

using namespace llvm;

namespace {
  struct ConstantsEncoder : public BasicBlockPass {
    ConstantsEncoder(Coder *c) : BasicBlockPass(ID), C(c) {}

    bool runOnBasicBlock(BasicBlock &BB) override;

    static char ID;
  private:
    Coder *C;
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

      uint64_t value = CI->getZExtValue();
      int64_t svalue = (int64_t)(value << 32) >> 32;
      int64_t res = svalue * C->getA();
      ConstantInt *CodedCI = ConstantInt::getSigned(dyn_cast<IntegerType>(C->getInt64Type()),
                                                    res);
      I->setOperand(i, CodedCI);

      modified = true;
    }
  }

  return modified;
}

Pass *createConstantsEncoder(Coder *c) {
  return new ConstantsEncoder(c);
}
