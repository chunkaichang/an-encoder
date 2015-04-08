
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
    Constant *buildEncodedConstant(Constant *);

    Coder *C;
  };
}

char GlobalsEncoder::ID = 0;

Constant *GlobalsEncoder::buildEncodedConstant(Constant *init) {
  Constant *result = init;
  Type *ty = init->getType();

  if (ty->isIntegerTy()) {
    if (const ConstantInt *ci = dyn_cast<ConstantInt>(init)) {
      uint64_t res = ci->getValue().getLimitedValue() * C->getA();
      // Programs for encoding are expected to operate on 64bit integers only,
      // so we also only handle 64bit integers here: (The main reason for not
      // having an assertion here is that strings, i.e. arrays with element
      // type 'i8', must not be encoded; yet strings are very common.)
      if (ci->getType() != C->getInt64Type())
        return result;

      result = ConstantInt::get(ci->getType(), res);
    }
  } else if (ty->isArrayTy()) {
    ArrayType *aty = dyn_cast<ArrayType>(ty);
    SmallVector<Constant*, 32> newElements;

    for (unsigned i = 0; i < aty->getNumElements(); i++) {
      Constant *newElt, *oldElt = init->getAggregateElement(i);
      newElt = buildEncodedConstant(oldElt);
      newElements.push_back(newElt);
    }

    result = ConstantArray::get(aty, newElements);
  }

  return result;
}

bool GlobalsEncoder::runOnModule(Module &M) {
  bool modified = false;

  for (auto I = M.global_begin(), E = M.global_end(); I != E; I++) {
    GlobalVariable *GV = &(*I);
    if (!GV->hasInitializer())
      continue;

    Constant *init = GV->getInitializer();
    GV->setInitializer(buildEncodedConstant(init));
    modified = true;
  }

  return modified;
}

Pass *createGlobalsEncoder(Coder *c) {
  return new GlobalsEncoder(c);
}
