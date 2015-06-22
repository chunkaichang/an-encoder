
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

#include "Coder.h"
#include "UsesVault.h"

using namespace llvm;

namespace {
  struct CallHandler : public BasicBlockPass {
    CallHandler(Coder *c) : BasicBlockPass(ID), C(c) {}

    bool runOnBasicBlock(BasicBlock &BB) override;

    static char ID;
  private:
    Coder *C;
  };
}

char CallHandler::ID = 0;

bool CallHandler::runOnBasicBlock(BasicBlock &BB) {
  bool modified = false;
  Module *M = BB.getParent()->getParent();
  LLVMContext &ctx = M->getContext();
  Type *int64Ty = Type::getInt64Ty(ctx);

  auto I = BB.begin(), E = BB.end();
  while (I != E) {
    auto N = std::next(I);

    CallInst *CI = dyn_cast<CallInst>(&(*I));
    if (!CI) {
      I = N;
      continue;
    }

    Function *add_with_overflow = Intrinsic::getDeclaration(M,
                                                            Intrinsic::sadd_with_overflow,
                                                            int64Ty);
    Function *F = CI->getCalledFunction();
    if (F == add_with_overflow
#if (defined(PRINTF_DEBUG) || defined(LOG_ACCU))
        || F->getName().startswith("printf")
        || F->getName().startswith("fprintf")
#endif
       ) {
      I = N;
      continue;
    }
    // Only operate on functions that are externally defined,
    // i.e. such functions for which only a declaration exists
    // within the current module:
    if (F->isDeclaration()) {
      // Do not operate on the encoding/decoding intrinsics:
      if (F->isIntrinsic() &&
          (F->getIntrinsicID() == Intrinsic::an_encode ||
           F->getIntrinsicID() == Intrinsic::an_decode ||
           F->getIntrinsicID() == Intrinsic::an_encode_value ||
           F->getIntrinsicID() == Intrinsic::an_decode_value ||
           F->getIntrinsicID() == Intrinsic::an_check  ||
           F->getIntrinsicID() == Intrinsic::an_signal ||
           F->getIntrinsicID() == Intrinsic::an_assert ||
           F->getIntrinsicID() == Intrinsic::an_placeholder)) {
        I = N;
        continue;
      }
      // Decode the arguments to external function calls:
      for (unsigned i = 0; i < CI->getNumArgOperands(); i++) {
        Value *Op = CI->getArgOperand(i);
        if (!Op->getType()->isIntegerTy() &&
        		!Op->getType()->isPointerTy())
          continue;
				if (Op->getType()->isPointerTy() &&
					  dyn_cast<GlobalValue>(Op->stripPointerCasts()))
					continue;

        Op = C->createEncRegionExit(Op, Op->getType(), CI);
        if (i < F->getFunctionType()->getNumParams()) {
          Type *paramTy = F->getFunctionType()->getParamType(i);
          if (paramTy->isIntegerTy() && paramTy != Op->getType())
            Op = C->createTrunc(Op, paramTy, CI);
        }

        CI->setArgOperand(i, Op);
        modified = true;
      }
      // Encode the return value from the external function call:
      Type *retTy = F->getReturnType();
      if (retTy->isIntegerTy() || retTy->isPointerTy()) {
        Value *ret = CI;
        if (ret->getType() == C->getInt64Type() ||
						ret->getType()->isPointerTy()) {
          // Save all current uses of the return value to the vault. These
          // are the uses that need to be replaced with the encoded return
          // value. (The 'createEncRegionEntry' method will give rise to
          // another use of the return value, which must not be replaced.)
          UsesVault UV(CI->uses());

          ret = C->createEncRegionEntry(ret, N);
          UV.replaceWith(ret);
        } else {
          for (Value::use_iterator u = CI->use_begin(), e = CI->use_end();
               u != e; ++u) {
            User *U = u->getUser();
            SExtInst *se = dyn_cast<SExtInst>(U);
            assert(se && "Value returned by external function call is not \
                          immediately extended to \'i64\'");
            if (se->getNumUses()) {
              UsesVault UV(se->uses());
              Instruction *insertPoint =
                dyn_cast<Instruction>(se->uses().begin()->getUser());
              assert(insertPoint && "Sign-extended return value is not used \
                                     by any instruction");
              Value *enc = C->createEncRegionEntry(se, insertPoint);
              UV.replaceWith(enc);
            }
          }
        }
        modified = true;
      }
    } else {
			// encode pointers to global variables if they are passed as arguments
      for (unsigned i = 0; i < CI->getNumArgOperands(); i++) {
        Value *Op = CI->getArgOperand(i);
				if (Op->getType()->isPointerTy() &&
					  dyn_cast<GlobalValue>(Op->stripPointerCasts())) {
        
					Op = C->createEncode(Op, CI);
        	CI->setArgOperand(i, Op);
        	modified = true;
				}
			}
		}
    I = N;
  }

  return modified;
}

Pass *createCallHandler(Coder *c) {
  return new CallHandler(c);
}
