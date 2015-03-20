
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Metadata.h"

#include <iostream>

#include "Coder.h"
#include "UsesVault.h"

using namespace llvm;

namespace {
  struct Int64Checker : public BasicBlockPass {
    Int64Checker(Coder *c, bool i1allowed) : BasicBlockPass(ID), C(c), i1(i1allowed) {}

    bool runOnBasicBlock(BasicBlock &BB) override;

    static char ID;
  private:
    Coder *C;
    const bool i1;
  };
}

char Int64Checker::ID = 0;

bool Int64Checker::runOnBasicBlock(BasicBlock &BB) {
  for (auto I = BB.begin(), E = BB.end(); I != E;
       ++I) {
    Instruction *inst = &(*I);
    for (unsigned i = 0; i < inst->getNumOperands(); i++) {
      Value *Op = inst->getOperand(i);
      Type *Ty = Op->getType();

      if (!Ty->isIntegerTy() ||
          Ty == C->getInt64Type())
        continue;

      if (i1 && Ty->getIntegerBitWidth() == 1)
        continue;

      if (dyn_cast<ConstantInt>(Op))
        continue;

      if (SExtInst *sei = dyn_cast<SExtInst>(inst)) {
        if (sei->getType() != C->getInt64Type()) {
          sei->dump();
          assert(0 && "Invalid sign extend instruction");
        }
      }else if (ZExtInst *zei = dyn_cast<ZExtInst>(inst)) {
        if (zei->getType() != C->getInt64Type()) {
          zei->dump();
          assert(0 && "Invalid zero extend instruction");
        }
      } else if (CallInst *ci = dyn_cast<CallInst>(inst)) {
        Function *F = ci->getCalledFunction();
        if (!F->isDeclaration()) {
          ci->dump();
          assert(0 && "Invalid argument to non-external call");
        }
      } else if (BranchInst *bi = dyn_cast<BranchInst>(inst)) {
        if (bi->isUnconditional() || i != 0) {
          bi->dump();
          assert(0 && "Non-64bit integer outside of branch condition");
        }
      } else if (SelectInst *si = dyn_cast<SelectInst>(inst)) {
        if (i != 0) {
          si->dump();
          assert(0 && "Non-64bit integer outside of select condition");
        }
      } else {
        inst->dump();
        assert(0 && "Found invalid use of non-64bit integer");
      }
    }
  }

  return false;
}

Pass *createInt64Checker(Coder *c, bool i1allowed) {
  return new Int64Checker(c, i1allowed);
}
