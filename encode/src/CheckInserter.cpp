
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

#include "Coder.h"

using namespace llvm;

namespace {
  struct CheckConstr {
    virtual Value *getCheckFun(Module *M) {
        LLVMContext &ctx = M->getContext();
        FunctionType *checkTy = FunctionType::get(Type::getVoidTy(ctx), false);
        Value *check = M->getOrInsertFunction("check_enc", checkTy);
        return check;
    }
  };

  struct CheckInserter : public BasicBlockPass, public CheckConstr {
    CheckInserter(Coder *c, bool before = true) : BasicBlockPass(ID), C(c), Before(before) {}

    bool runOnBasicBlock(BasicBlock &BB) override;

    static char ID;

  private:
    bool Before;
    Coder *C;
  };
  
  struct BBCheckInserter : public BasicBlockPass, public CheckConstr {
    BBCheckInserter() : BasicBlockPass(ID) {}

    bool runOnBasicBlock(BasicBlock &BB) override;

    static char ID;
  };
  
  struct FunCheckInserter : public FunctionPass, public CheckConstr {
    FunCheckInserter() : FunctionPass(ID) {}

    bool runOnFunction(Function &F) override;

    static char ID;
  };
}

char CheckInserter::ID = 0;

bool CheckInserter::runOnBasicBlock(BasicBlock &BB) {
  bool modified = false;
  Module *M = BB.getParent()->getParent();

  auto I = BB.begin(), E = BB.end();
  while( I != E) {
    auto N = std::next(I);
    CallInst *ci = dyn_cast<CallInst>(I);
    Function *callee = ci ? ci->getCalledFunction() : nullptr;
    if (callee && callee->getName().equals("accumulate_enc")) {
	IRBuilder<> builder(&BB);
	if (Before) C->createAssertOnAccu(I);
	else C->createAssertOnAccu(N);
        modified = true;
    }
    I = N;
  }

  return modified;
}

Pass *createCheckInserter(Coder *c, bool before=true) {
  return new CheckInserter(c, before);
}


char BBCheckInserter::ID = 0;

bool BBCheckInserter::runOnBasicBlock(BasicBlock &BB) {
  Module *M = BB.getParent()->getParent();

  Instruction *term = BB.getTerminator();
  assert(term && "Basic block not well-formed");
  IRBuilder<> builder(term);
  builder.CreateCall(getCheckFun(M));
  return true;
}

Pass *createBBCheckInserter() {
  return new BBCheckInserter();
}


char FunCheckInserter::ID = 0;

bool FunCheckInserter::runOnFunction(Function &F) {
  Module *M = F.getParent();

  for (Function::iterator i = F.begin(), e = F.end(); i != e; ++i) {
    BasicBlock &BB = *i;
    TerminatorInst *ti = BB.getTerminator();
    assert(ti && "Basic block not well-formed");
    if (ReturnInst *ri = dyn_cast<ReturnInst>(ti)) {
      IRBuilder<> builder(ti);
      builder.CreateCall(getCheckFun(M));
    }
  }
  return true;
}

Pass *createFunCheckInserter() {
  return new FunCheckInserter();
}
