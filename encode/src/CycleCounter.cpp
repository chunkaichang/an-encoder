
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

// TODO: suspend preemption/interrupts for accurate counting
// of cycles.

namespace {
  struct CycleCounter : public ModulePass {
    CycleCounter() : ModulePass(ID) {}

    bool runOnModule(Module &M) override;

    static char ID;
  };
}

char CycleCounter::ID = 0;

bool CycleCounter::runOnModule(Module &M) {
  Function *main = M.getFunction("main");
  if (main == nullptr)
    return false;

  Type *int64Ty = Type::getInt64Ty(M.getContext());
  FunctionType *rdtscTy = FunctionType::get(int64Ty, false);
  Constant *rdtsc = M.getOrInsertFunction("rdtsc", rdtscTy);

  Type *voidTy  = Type::getVoidTy(M.getContext());
  FunctionType *msgTy = FunctionType::get(voidTy, int64Ty, false);
  Constant *msg   = M.getOrInsertFunction("cycles_msg", msgTy);

  // Insert code to obtain the starting number of cycles:
  BasicBlock &EB = main->getEntryBlock();
  Value *start = CallInst::Create(rdtsc, "", EB.begin());

  // Find all basic blocks that return from "main" and insert
  // code to obtain the final number of cycles. Also insert a
 // call to "cycles_msg" to print the number of cyces counted:
  for (auto I = main->begin(), E = main->end(); I != E; ++I) {
    Instruction *term = I->getTerminator();
    if (!isa<ReturnInst>(term))
      continue;

    IRBuilder<> builder(term);
    Value *end = builder.CreateCall(rdtsc, "");
    Value *cycles = builder.CreateSub(end, start);
    builder.CreateCall(msg, cycles);
  }

  return true;
}

static RegisterPass<CycleCounter> X("CycleCounter",
                                    "",
                                    false,
                                    false);

Pass *createCycleCounter() {
  return new CycleCounter();
}
