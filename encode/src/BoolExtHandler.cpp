
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

#include <set>

#include "Coder.h"
#include "UsesVault.h"

using namespace llvm;

namespace {
  struct BoolExtHandler : public BasicBlockPass {
    BoolExtHandler(Coder *c) : BasicBlockPass(ID), C(c) {}

    bool runOnBasicBlock(BasicBlock &BB) override;

    static char ID;
  private:
    Coder *C;
  };
}

char BoolExtHandler::ID = 0;

bool BoolExtHandler::runOnBasicBlock(BasicBlock &BB) {
  bool modified = false;
  auto I = BB.begin(), E = BB.end();

  while(I != E) {
    auto N = std::next(I);

    if (!isa<ZExtInst>(&(*I)) && !isa<SExtInst>(&(*I))) {
      I = N;
      continue;
    }
    CastInst *ci = dyn_cast<CastInst>(&(*I));
    Type *srcTy = ci->getSrcTy();
    if (!srcTy->isIntegerTy(1)) {
      I = N;
      continue;
    }

    // Now find the final 'ZExt' or 'SExt' to 'i64':
    std::set<Value*> worklist;
    worklist.insert(&(*I));

    while (!worklist.empty()) {
      for (auto wli = worklist.begin(), wle = worklist.end(); wli != wle; ++wli) {
        worklist.erase(wli);
        Value *wlv = *wli;

        for (auto ui = wlv->use_begin(), ue = wlv->use_end(); ui != ue; ++ui) {
          User *user = ui->getUser();
          if (!isa<ZExtInst>(user) && !isa<SExtInst>(user))
            continue;

          CastInst *ci = dyn_cast<CastInst>(user);
          Type *destTy = ci->getDestTy();
          if (destTy != C->getInt64Type()) {
            worklist.insert(user);
          } else {
            UsesVault UV(ci->uses());
            BasicBlock::iterator CI(ci);
            Value *enc = C->createEncode(ci, std::next(CI));
            UV.replaceWith(enc);
            modified = true;
          }
        }
      }
    }

    I = N;
  }

  return modified;
}

Pass *createBoolExtHandler(Coder *c) {
  return new BoolExtHandler(c);
}
