
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

namespace {
  struct GlobalsEncoder : public ModulePass {
    GlobalsEncoder(const GlobalVariable *a) : ModulePass(ID), A(a) {}
    GlobalsEncoder() : GlobalsEncoder(NULL) {}

    bool runOnModule(Module &M) override;

    static char ID;
  private:
    const GlobalVariable *A;
  };
}

char GlobalsEncoder::ID = 0;

bool GlobalsEncoder::runOnModule(Module &M) {
  bool modified = false;

  if (A == nullptr)
    return modified;

  for (auto I = M.global_begin(), E = M.global_end(); I != E; I++) {
    GlobalVariable *GV = &(*I);
    if (!GV->hasInitializer())
      continue;

    const ConstantInt *variableValue
      = dyn_cast<ConstantInt>(GV->getInitializer());
    if (!variableValue)
      continue;

    const ConstantInt *codeValue
      = dyn_cast<ConstantInt>(A->getInitializer());
    assert(codeValue);

    const APInt &value = variableValue->getValue();
    const APInt &code  = codeValue->getValue();
    uint64_t res = value.getLimitedValue() * code.getLimitedValue();
    ConstantInt *C = ConstantInt::get(variableValue->getType(), res);
    GV->setInitializer(C);

    modified = true;
  }

  return modified;
}

static RegisterPass<GlobalsEncoder> X("GlobalsEncoder",
                                      "",
                                      false,
                                      false);

Pass *createGlobalsEncoder(const GlobalVariable *a) {
  return new GlobalsEncoder(a);
}
