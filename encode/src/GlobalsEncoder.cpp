
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

    Constant *init = GV->getInitializer();
    if (const ConstantInt *ci = dyn_cast<ConstantInt>(init)) {
      uint64_t res = ci->getValue().getLimitedValue() * C->getA();
      assert(ci->getType() == C->getInt64Type() &&
             "Unexpected non-64bit integer type");
      ConstantInt *enc = ConstantInt::get(ci->getType(), res);
      GV->setInitializer(enc);

      modified = true;
    } else if (const ConstantDataArray *cda = dyn_cast<ConstantDataArray>(init)) {
      // We only handle arrays whose elements are 64bit integers: (The main reason
      // for doing this is that strings, i.e. arrays with element type 'i8', must
      // not be encoded; yet strings are very common.)
      if (cda->getElementType() != C->getInt64Type())
        continue;

      assert(cda->getElementType() == C->getInt64Type() &&
             "Unexpected non-64bit integer type");

      SmallVector<uint64_t, 32> result;
      for (unsigned i = 0; i < cda->getNumElements(); i++) {
        uint64_t res = cda->getElementAsInteger(i) * C->getA();
        result.push_back(res);
      }
      LLVMContext &ctx = M.getContext();
      GV->setInitializer(ConstantDataArray::get(ctx, result));

      modified = true;
    }
  }

  return modified;
}

Pass *createGlobalsEncoder(Coder *c) {
  return new GlobalsEncoder(c);
}
