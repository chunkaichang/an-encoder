
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
  struct ConstantsEncoder : public BasicBlockPass {
    ConstantsEncoder(const GlobalVariable *a) : BasicBlockPass(ID), A(a) {}
    ConstantsEncoder() : ConstantsEncoder(NULL) {}

    bool runOnBasicBlock(BasicBlock &BB) override;

    static char ID;
  private:
    const GlobalVariable *A;
  };
}

char ConstantsEncoder::ID = 0;

bool ConstantsEncoder::runOnBasicBlock(BasicBlock &BB) {
  bool modified = false;

  if (A == nullptr)
    return modified;

  for (auto I = BB.begin(), E = BB.end(); I != E; I++) {
    if (isa<AllocaInst>(&(*I)))
      continue;

    for (unsigned i = 0; i < I->getNumOperands(); i++) {
      Value *Op = I->getOperand(i);
      ConstantInt *CI = dyn_cast<ConstantInt>(Op);
      if (!CI)
        continue;

      const ConstantInt *codeValue
        = dyn_cast<ConstantInt>(A->getInitializer());
      assert(codeValue);

      const APInt &value = CI->getValue();
      const APInt &code  = codeValue->getValue();
      uint64_t res = value.getLimitedValue() * code.getLimitedValue();
      ConstantInt *C = ConstantInt::get(CI->getType(), res);
      I->setOperand(i, C);

      modified = true;
    }
  }

  return modified;
}

static RegisterPass<ConstantsEncoder> X("ConstantsEncoder",
                                         "",
                                         false,
                                         false);

Pass *createConstantsEncoder(const GlobalVariable *a) {
  return new ConstantsEncoder(a);
}
