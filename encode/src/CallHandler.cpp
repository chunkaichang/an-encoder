
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Metadata.h"

#include <iostream>

using namespace llvm;

namespace {
  struct CallHandler : public BasicBlockPass {
    CallHandler(GlobalVariable *a) : BasicBlockPass(ID), A(a) {}
    CallHandler() : CallHandler(NULL) {}

    bool runOnBasicBlock(BasicBlock &BB) override;

    static char ID;
  private:
    GlobalVariable *A;
  };
}

char CallHandler::ID = 0;

bool CallHandler::runOnBasicBlock(BasicBlock &BB) {
  bool modified = false;
  Module *M = BB.getParent()->getParent();
  LLVMContext &ctx = M->getContext();
  Type *int64Ty = Type::getInt64Ty(ctx);

  if (A == nullptr)
    return modified;

  auto I = BB.begin(), E = BB.end();
  while (I != E) {
    auto N = std::next(I);

    CallInst *CI = dyn_cast<CallInst>(&(*I));
    if (!CI) {
      I = N;
      continue;
    }

    Function *F = CI->getCalledFunction();
    // Only operate on functions that are externally defined,
    // i.e. such functions for which only a declaration exists
    // within the current module:
    if (F->isDeclaration()) {
      // Do not operate on the encoding/decoding intrinsics:
      if (F->isIntrinsic() &&
          (F->getIntrinsicID() == Intrinsic::an_encode ||
           F->getIntrinsicID() == Intrinsic::an_decode ||
           F->getIntrinsicID() == Intrinsic::an_check  ||
           F->getIntrinsicID() == Intrinsic::an_signal ||
           F->getIntrinsicID() == Intrinsic::an_assert ||
           F->getIntrinsicID() == Intrinsic::an_placeholder)) {
        I = N;
        continue;
      }
      // Decode the arguments to external function calls:
      for (unsigned i = 0; i < CI->getNumArgOperands(); i++) {
        Value *NewOp, *Op = CI->getArgOperand(i);
        if (!Op->getType()->isIntegerTy() ||
            Op->getType()->getIntegerBitWidth() == 1)
          continue;

        unsigned w = Op->getType()->getIntegerBitWidth();
        if (w < 64)
          NewOp = new SExtInst(Op, int64Ty, "", &(*I));
        else if (w > 64)
          NewOp = new TruncInst(Op, int64Ty, "", &(*I));
        else
          NewOp = Op;
        assert(NewOp->getType() == int64Ty);

        ConstantInt *codeValue
          = dyn_cast<ConstantInt>(A->getInitializer());
        assert(codeValue);
        const APInt &code  = codeValue->getValue();
        Value *a = ConstantInt::get(NewOp->getType(), code.getLimitedValue());

        IRBuilder<> builder(CI);
        Value *decode =
          Intrinsic::getDeclaration(M,
                                    Intrinsic::an_decode,
                                    NewOp->getType());
        Value *res = builder.CreateCall2(decode, NewOp, a);
        if (w < 64)
          res = new TruncInst(res, Op->getType(), "", &(*I));
        else if (w > 64)
          res = new SExtInst(res, Op->getType(), "", &(*I));
        CI->setArgOperand(i, res);

        modified = true;
      }
      // Encode the return value from the external function call:
      Type *retTy = F->getReturnType();
      if (retTy->isIntegerTy()) {
        // Save all current uses of the return value; these are the
        // uses that need to be replaced with the encoded return value.
        // (The encoding operation that we are about to create will
        // give rise to another use of the return value, which must not
        // be replaced. Hence we cannot use 'replaceAllUsesWith'.)
        auto uses = CI->uses();
        Value *ret = CI;
        unsigned w = retTy->getIntegerBitWidth();
        if (w < 64)
          ret = new SExtInst(ret, int64Ty, "", &(*N));
        else if (w > 64)
          ret = new TruncInst(ret, int64Ty, "", &(*N));
        assert(ret->getType() == int64Ty);

        ConstantInt *codeValue
          = dyn_cast<ConstantInt>(A->getInitializer());
        assert(codeValue);
        const APInt &code  = codeValue->getValue();
        Value *a = ConstantInt::get(int64Ty, code.getLimitedValue());

        IRBuilder<> builder(N);
        Value *encode =
          Intrinsic::getDeclaration(M,
                                    Intrinsic::an_encode,
                                    int64Ty);
        ret = builder.CreateCall2(encode, ret, a);

        if (w < 64)
          ret = new TruncInst(ret, retTy, "", &(*N));
        else if (w > 64)
          ret = new SExtInst(ret, retTy, "", &(*N));
        // Here we cannot simply use a call to 'replaceAllUsesWith
        // (see comment above, at the beginning of this if block):
        auto i = uses.begin(), e = uses.end();
        while (i != e) {
          auto n = std::next(i);
          i->set(ret);
          i = n;
        }

        modified = true;
      }
      // TODO: The following block is for encoding pointers that
      // are returned from external function calls. Currently we
      // do not encode pointers (unless they are cast to integers),
      // so the following block is commented out:
      /*else if (retTy->isPointerTy()) {
        auto uses = CI->uses();
        Value *ret = CI;

        IRBuilder<> builder(N);
        Value *PToI = builder.CreatePtrToInt(ret, int64Ty);

        ConstantInt *codeValue
          = dyn_cast<ConstantInt>(A->getInitializer());
        assert(codeValue);
        const APInt &code  = codeValue->getValue();
        Value *a = ConstantInt::get(int64Ty, code.getLimitedValue());

        Value *encode =
          Intrinsic::getDeclaration(M,
                                    Intrinsic::an_encode,
                                    int64Ty);
        ret = builder.CreateCall2(encode, PToI, a);
      }*/
    }
    I = N;
  }

  return modified;
}

static RegisterPass<CallHandler> X("CallHandler",
                                   "",
                                   false,
                                   false);

Pass *createCallHandler(GlobalVariable *a) {
  return new CallHandler(a);
}
