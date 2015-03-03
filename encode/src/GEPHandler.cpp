
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
  struct GEPHandler : public BasicBlockPass {
    GEPHandler(GlobalVariable *a) : BasicBlockPass(ID), A(a) {}
    GEPHandler() : GEPHandler(NULL) {}

    bool runOnBasicBlock(BasicBlock &BB) override;

    static char ID;
  private:
    GlobalVariable *A;
  };
}

char GEPHandler::ID = 0;

bool GEPHandler::runOnBasicBlock(BasicBlock &BB) {
  bool modified = false;
  Module *M = BB.getParent()->getParent();

  if (A == nullptr)
    return modified;

  for (auto I = BB.begin(), E = BB.end(); I != E; I++) {
    GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(&(*I));
    if (!GEP)
      continue;

    IRBuilder<> builder(&(*I));

    for (unsigned i = 0; i < I->getNumOperands(); i++) {
      Value *Op = I->getOperand(i);
      if (!Op->getType()->isIntegerTy())
        continue;

      Value *decode =
        Intrinsic::getDeclaration(M,
                                  Intrinsic::an_decode,
                                  Op->getType());
      const ConstantInt *codeValue
        = dyn_cast<ConstantInt>(A->getInitializer());
      assert(codeValue);
      const APInt &code  = codeValue->getValue();
      Value *a = ConstantInt::get(Op->getType(), code.getLimitedValue());
      Value *NewOp = builder.CreateCall2(decode, Op, a);
      I->setOperand(i, NewOp);

      modified = true;
    }
  }

  return modified;
}

static RegisterPass<GEPHandler> X("GEPHandler",
                                  "",
                                  false,
                                  false);

Pass *createGEPHandler(GlobalVariable *a) {
  return new GEPHandler(a);
}
