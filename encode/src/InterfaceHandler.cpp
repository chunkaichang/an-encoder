
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Constants.h"

#include "Coder.h"
#include "UsesVault.h"

using namespace llvm;

namespace {
  struct InterfaceHandler: public FunctionPass {
    InterfaceHandler(Coder *c) : FunctionPass(ID), C(c) {}

    bool runOnFunction(Function &F) override;

    static char ID;
  private:
    Coder *C;
  };
}

char InterfaceHandler::ID = 0;

bool InterfaceHandler::runOnFunction(Function &F) {
  if (!F.getName().startswith_lower("___enc_"))
    return false;

  for (Function::arg_iterator a = F.arg_begin(), e = F.arg_end(); a != e; ++a) {
    if (a->getType()->isIntegerTy()) {
      UsesVault UV(a->uses());
      Value *arg = a;
      Instruction *insertPt = F.getEntryBlock().begin();
      Value *enc = C->createEncRegionEntry(arg, insertPt);
      UV.replaceWith(enc);
    }
  }

  if (F.getReturnType()->isIntegerTy()) {
    for (Function::iterator i = F.begin(), e = F.end(); i != e; ++i) {
      BasicBlock &BB = *i;
      TerminatorInst *ti = BB.getTerminator();
      if (ReturnInst *ri = dyn_cast<ReturnInst>(ti)) {
        Value *enc = C->createEncRegionExit(ri->getOperand(0), F.getReturnType(), ri);
        ri->setOperand(0, enc);
      }
    }
  }
  return true;
}

Pass *createInterfaceHandler(Coder *c) {
  return new InterfaceHandler(c);
}
