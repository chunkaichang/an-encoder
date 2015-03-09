
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Metadata.h"

using namespace llvm;

namespace {
  struct GEPHandler : public BasicBlockPass {
    GEPHandler(GlobalVariable *a) : BasicBlockPass(ID), A(a) {}
    GEPHandler() : GEPHandler(NULL) {}

    bool runOnBasicBlock(BasicBlock &BB) override;

    static char ID;
  private:
    GlobalVariable *A;
  };
}

char GEPHandler::ID = 0;

bool GEPHandler::runOnBasicBlock(BasicBlock &BB) {
  bool modified = false;
  Module *M = BB.getParent()->getParent();

  if (A == nullptr)
    return modified;

  for (auto I = BB.begin(), E = BB.end(); I != E; I++) {
    const ConstantInt *codeValue
      = dyn_cast<ConstantInt>(A->getInitializer());
    assert(codeValue);
    const APInt &code  = codeValue->getValue();

    // TODO: The code would perhaps become clearer if
    // 'isa_cast<...>' instructions were used in the if
    // clauses instead of these casts:
    GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(&(*I));
    PtrToIntInst *PToI = dyn_cast<PtrToIntInst>(&(*I));
    IntToPtrInst *IToP = dyn_cast<IntToPtrInst>(&(*I));

    if (GEP) {
      for (unsigned i = 0; i < I->getNumOperands(); i++) {
        Value *Op = I->getOperand(i);
        if (!Op->getType()->isIntegerTy())
          continue;

        Value *decode =
          Intrinsic::getDeclaration(M,
                                    Intrinsic::an_decode,
                                    Op->getType());
        Value *a = ConstantInt::get(Op->getType(), code.getLimitedValue());
        IRBuilder<> builder(&(*I));
        Value *NewOp = builder.CreateCall2(decode, Op, a);
        I->setOperand(i, NewOp);

        modified = true;
      }
    // TODO: It would perhaps be logically cleaner if handling of
    // 'ptrtoint' instructions were moved to the 'OperationsEncoder':
    } else if (PToI) {
      // Save all current uses of the 'ptrtoint' instruction, to be
      // replaced with the encoded integer value: (The encoding
      // operation we are about to create will generate another use
      // of the 'ptrtoint' instruction's result. Therefore we cannot
      // simply call 'replaceAllUsesWith' at the end of this block.)
      auto uses = PToI->uses();
      Value *encode =
        Intrinsic::getDeclaration(M,
                                  Intrinsic::an_encode,
                                  PToI->getType());
      Value *a = ConstantInt::get(PToI->getType(), code.getLimitedValue());
      IRBuilder<> builder(&(*std::next(I)));
      Value *NewInt = builder.CreateCall2(encode, PToI, a);
      // Replace all previous uses of the 'ptrtoint' instruction's
      // result with the encoded value:
      auto i = uses.begin(), e = uses.end();
      while (i != e) {
        auto n = std::next(i);
        i->set(NewInt);
        i = n;
      }
    // TODO: It would perhaps be logically cleaner if handling of
    // 'inttoptr' instructions were moved to the 'OperationsEncoder':
    } else if (IToP) {
      Value *Op = IToP->getOperand(0);
      Value *decode =
        Intrinsic::getDeclaration(M,
                                  Intrinsic::an_decode,
                                  Op->getType());
      Value *a = ConstantInt::get(Op->getType(), code.getLimitedValue());
      IRBuilder<> builder(&(*I));
      Value *NewInt = builder.CreateCall2(decode, Op, a);
      IToP->setOperand(0, NewInt);
    }
  }

  return modified;
}

static RegisterPass<GEPHandler> X("GEPHandler",
                                  "",
                                  false,
                                  false);

Pass *createGEPHandler(GlobalVariable *a) {
  return new GEPHandler(a);
}
