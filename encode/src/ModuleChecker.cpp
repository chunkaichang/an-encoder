
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"

#include "Coder.h"

using namespace llvm;

namespace {
  struct ModuleChecker : public ModulePass {
    ModuleChecker(Coder *c, bool i1allowed) : ModulePass(ID), C(c), i1Allowed(i1allowed) {}

    bool runOnModule(Module &M) override;

    void checkFunction(const Function &F);
    void checkInstruction(const Instruction &I);

    static char ID;
  private:
    Coder *C;
    const bool i1Allowed;
  };
}

char ModuleChecker::ID = 0;

void ModuleChecker::checkInstruction(const Instruction &I) {
  const Instruction *inst = &I;

  for (unsigned i = 0; i < inst->getNumOperands(); i++) {
    Value *Op = inst->getOperand(i);
    Type *Ty = Op->getType();

    if (!Ty->isIntegerTy() ||
        Ty == C->getInt64Type())
      continue;

    if (i1Allowed && Ty->getIntegerBitWidth() == 1)
      continue;

    // Check for allowed exceptions, i.e. places where non-64bit integers
    // are acceptable:
    if (const SExtInst *sei = dyn_cast<SExtInst>(inst)) {
      if (sei->getType() != C->getInt64Type()) {
        sei->dump();
        assert(0 && "Invalid sign extend instruction");
      }
    } else if (const ZExtInst *zei = dyn_cast<ZExtInst>(inst)) {
      if (zei->getType() != C->getInt64Type()) {
        zei->dump();
        assert(0 && "Invalid zero extend instruction");
      }
    } else if (const CallInst *ci = dyn_cast<CallInst>(inst)) {
      Function *F = ci->getCalledFunction();
      if (!F->isDeclaration()) {
        ci->dump();
        assert(0 && "Invalid argument to non-external call");
      }
    } else if (const BranchInst *bi = dyn_cast<BranchInst>(inst)) {
      if (bi->isUnconditional() || i != 0) {
        bi->dump();
        assert(0 && "Non-64bit integer outside of branch condition");
      }
    } else if (const SelectInst *si = dyn_cast<SelectInst>(inst)) {
      if (i != 0) {
        si->dump();
        assert(0 && "Non-64bit integer outside of select condition");
      }
    } else if (const AllocaInst *ai = dyn_cast<AllocaInst>(inst)) {
      if (i != 0) {
        ai->dump();
        assert(0 && "Non-64bit integer in alloca instruction after 1st argument");
      }
    } else if (const GetElementPtrInst *gepi = dyn_cast<GetElementPtrInst>(inst)) {
      if (i == 0) {
        gepi->dump();
        assert(0 && "Found integer as the first argument of a GEP instruction");
      }
    } else {
      inst->dump();
      assert(0 && "Found invalid use of non-64bit integer");
    }
  }

}

void ModuleChecker::checkFunction(const Function &F) {
  for (auto BBI = F.begin(), BBE = F.end(); BBI != BBE; ++BBI) {
    for (auto I = BBI->begin(), E = BBI->end(); I != E; ++I) {
      checkInstruction(*I);
    }
  }
}

bool ModuleChecker::runOnModule(Module &M) {
  Function *main = M.getFunction("main");
  if (main) {
    assert(0 && "Cannot encode main module");
  }

  for (auto fi = M.begin(), fe = M.end(); fi != fe; ++fi)
    checkFunction(*fi);

  return false;
}

Pass *createModuleChecker(Coder *c, bool i1allowed) {
  return new ModuleChecker(c, i1allowed);
}
