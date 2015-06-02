
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
  struct BBCheckInserter : public BasicBlockPass {
    BBCheckInserter(Coder *c) : BasicBlockPass(ID), C(c) {}

    bool runOnBasicBlock(BasicBlock &BB) override;

    static char ID;
  private:
    Coder *C;
  };
  
  struct FunCheckInserter : public FunctionPass {
    FunCheckInserter(Coder *c) : FunctionPass(ID), C(c) {}

    bool runOnFunction(Function &F) override;

    static char ID;
  private:
    Coder *C;
  };
}

char BBCheckInserter::ID = 0;

bool BBCheckInserter::runOnBasicBlock(BasicBlock &BB) {
  Function *F = BB.getParent();
  // Do not insert checks in functions from the 'anlib' library:
  if (F->getName().endswith_lower("_enc"))
    return false;
  Module *M = BB.getParent()->getParent();

  Instruction *term = BB.getTerminator();
  assert(term && "Basic block not well-formed");
  C->createAssertOnAccu(term);
  return true;
}

Pass *createBBCheckInserter(Coder *c) {
  return new BBCheckInserter(c);
}


char FunCheckInserter::ID = 0;

bool FunCheckInserter::runOnFunction(Function &F) {
  // Do not insert checks in functions from the 'anlib' library:
  if (F.getName().endswith_lower("_enc"))
    return false;

  Module *M = F.getParent();

  for (Function::iterator i = F.begin(), e = F.end(); i != e; ++i) {
    BasicBlock &BB = *i;
    TerminatorInst *ti = BB.getTerminator();
    assert(ti && "Basic block not well-formed");
    if (ReturnInst *ri = dyn_cast<ReturnInst>(ti)) {
      C->createAssertOnAccu(ri);
    }
  }
  return true;
}

Pass *createFunCheckInserter(Coder *c) {
  return new FunCheckInserter(c);
}
