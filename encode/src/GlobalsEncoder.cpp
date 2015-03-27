
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"

#include "Coder.h"

using namespace llvm;

namespace {
  struct GlobalsEncoder : public ModulePass {
    GlobalsEncoder(Coder *c) : ModulePass(ID), C(c) {}

    bool runOnModule(Module &M) override;

    static char ID;
  private:
    Coder *C;
  };
}

char GlobalsEncoder::ID = 0;

bool GlobalsEncoder::runOnModule(Module &M) {
  bool modified = false;

  for (auto I = M.global_begin(), E = M.global_end(); I != E; I++) {
    GlobalVariable *GV = &(*I);
    if (!GV->hasInitializer())
      continue;

    const ConstantInt *init
      = dyn_cast<ConstantInt>(GV->getInitializer());
    if (!init)
      continue;

    const APInt &initInt = init->getValue();
    uint64_t res = initInt.getLimitedValue() * C->getA();
    assert(init->getType() == C->getInt64Type() &&
           "Unexpected non-64bit integer type");
    ConstantInt *enc = ConstantInt::get(init->getType(), res);
    GV->setInitializer(enc);

    modified = true;
  }

  return modified;
}

Pass *createGlobalsEncoder(Coder *c) {
  return new GlobalsEncoder(c);
}
