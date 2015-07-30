
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Constants.h"

#include "coder/ProfiledCoder.h"
#include "coder/UsesVault.h"

using namespace llvm;

namespace {
  struct InterfaceHandler: public FunctionPass {
    InterfaceHandler(ProfiledCoder *pc) : FunctionPass(ID), PC(pc) {}

    bool runOnFunction(Function &F) override;

    static char ID;
  private:
    ProfiledCoder *PC;
  };
}

char InterfaceHandler::ID = 0;

bool InterfaceHandler::runOnFunction(Function &F) {
  if (!F.getName().startswith_lower("___enc_"))
    return false;

  for (Function::arg_iterator a = F.arg_begin(), e = F.arg_end(); a != e; ++a) {
      UsesVault UV(a->uses());
      Value *arg = a;
      Instruction *insertPt = F.getEntryBlock().begin();
      Value *enc = PC->createEncode(arg, insertPt);
      UV.replaceWith(enc);
  }

    for (Function::iterator i = F.begin(), e = F.end(); i != e; ++i) {
      BasicBlock &BB = *i;
      TerminatorInst *ti = BB.getTerminator();
      if (ReturnInst *ri = dyn_cast<ReturnInst>(ti)) {
        if (ri->getNumOperands()) {
          Value *enc = PC->createDecode(ri->getOperand(0), ri);
          enc = PC->createTrunc(enc, F.getReturnType(), ri);
          ri->setOperand(0, enc);
        }
      }
  }
  return true;
}

Pass *createInterfaceHandler(ProfiledCoder *pc) {
  return new InterfaceHandler(pc);
}
