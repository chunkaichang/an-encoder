
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
  struct AllocaHandler : public BasicBlockPass {
    AllocaHandler(Coder *c) : BasicBlockPass(ID), C(c) {}

    bool runOnBasicBlock(BasicBlock &BB) override;

    static char ID;
  private:
    Coder *C;
  };
}

char AllocaHandler::ID = 0;

bool AllocaHandler::runOnBasicBlock(BasicBlock &BB) {
  bool modified = false;
  Module *M = BB.getParent()->getParent();

  auto I = BB.begin(), E = BB.end();
  while (I != E) {
    auto N = std::next(I);

    AllocaInst *AI = dyn_cast<AllocaInst>(&(*I));

    if (AI) {
      Value *res = AI;
      Type  *resTy = AI->getType();
      Type  *allocTy = AI->getAllocatedType();
      Value *arraySize = AI->getArraySize();
      unsigned align = AI->getAlignment();

      if (allocTy->isIntegerTy()) {
        Value *newAI = new AllocaInst(C->getInt64Type(), arraySize, "", AI);

        for (auto u = AI->use_begin(), e = AI->use_end(); u != e;
             ++u) {
          Instruction *user = dyn_cast<Instruction>(u->getUser());
          assert(user);
          if (LoadInst *LI = dyn_cast<LoadInst>(user)) {
            Value *newLI = new LoadInst(newAI, "", user);

            if (LI->getType() != C->getInt64Type())
              newLI = new TruncInst(newLI, LI->getType(), "", user);

            LI->replaceAllUsesWith(newLI);
          }
        }
        res->replaceAllUsesWith(newAI);
        AI->eraseFromParent();
        modified = true;
      }
    }
    I = N;
  }
  BB.dump();

  return modified;
}

Pass *createAllocaHandler(Coder *c) {
  return new AllocaHandler(c);
}
