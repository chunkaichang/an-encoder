
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
#include "UsesVault.h"

#include <iostream>

using namespace llvm;

// TODO: suspend preemption/interrupts for accurate counting
// of cycles.

namespace {
  struct MainArgsHandler: public ModulePass {
    MainArgsHandler(Coder *c) : ModulePass(ID), C(c) {}

    bool runOnModule(Module &M) override;

    static char ID;
  private:
    Coder *C;
  };
}

char MainArgsHandler::ID = 0;

bool MainArgsHandler::runOnModule(Module &M) {
  Function *main = M.getFunction("main");
  if (main == nullptr)
    return false;

  for(Function::arg_iterator a = main->arg_begin(), e = main->arg_end(); a != e; ++a) {
    if (a->getType()->isIntegerTy()) {
      UsesVault UV(a->uses());
      Value *arg = a;
      Instruction *insertPt = main->getEntryBlock().begin();
      Value *enc = C->createEncRegionEntry(arg, insertPt);
      UV.replaceWith(enc);
    }
  }
  return true;
}

Pass *createMainArgsHandler(Coder *c) {
  return new MainArgsHandler(c);
}
