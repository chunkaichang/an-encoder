
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
  struct OperationsExpander : public FunctionPass {
    OperationsExpander() : FunctionPass(ID) {}

    bool runOnFunction(Function &F) override;

    static char ID;
  };
}

char OperationsExpander::ID = 0;

bool OperationsExpander::runOnFunction(Function &F) {
  LLVMContext &ctx = F.getContext();
  Module *M = F.getParent();

  bool m = true, modified = false;
  // Loop until no more modifications are made to the function:
  // ('m' indicates if the previous pass has made modifications,
  // hence 'm' start out true. Looping is necessary since expanding
  // the assert and signal intrinsics creates new basic blocks.)
  while (m) {
    m = false;
    auto BB = F.begin(), BE = F.end();
    // Iterate over the basics blocks in 'F':
    while( BB != BE) {
      auto BN = std::next(BB);
      auto I = BB->begin(), E = BB->end();
      // Iterate over instructions in basic block 'BB':
      while( I != E) {
        auto N = std::next(I);
        // We are only interested in call instructions, in particular
        // such instructions which call the "AN coding" intrinsics:
        CallInst *ci = dyn_cast<CallInst>(I);
        Function *callee = ci ? ci->getCalledFunction() : nullptr;

        if (callee && callee->isIntrinsic()) {
          switch (callee->getIntrinsicID()) {
          default: break;
#define HANDLE_REPLACE_OPERATION(intrinsic, operation)  \
          case Intrinsic::intrinsic: {                  \
            Value *x = ci->getArgOperand(0);            \
            Value *a = ci->getArgOperand(1);            \
                                                        \
            IRBuilder<> builder(ci);                    \
            Value *res = builder.Create ## operation(x, a); \
            ci->replaceAllUsesWith(res);                    \
            ci->eraseFromParent();                          \
            m = modified = true;                            \
            break;                                          \
          }

          HANDLE_REPLACE_OPERATION(an_encode, Mul)
          HANDLE_REPLACE_OPERATION(an_decode, SDiv)

          case Intrinsic::an_check: {
            Value *x = ci->getArgOperand(0);
            Value *a = ci->getArgOperand(1);
            IRBuilder<> builder(ci);
            Value *res = builder.CreateURem(x, a);
            ci->replaceAllUsesWith(res);
            ci->eraseFromParent();
            break;
          }
          case Intrinsic::an_signal:
          case Intrinsic::an_assert: {
            unsigned ID = callee->getIntrinsicID();
            Value *x = ci->getArgOperand(0);
            Value *a = ci->getArgOperand(1);

            BasicBlock *splitBB = BB->splitBasicBlock(I),
                       *trapBB  = BasicBlock::Create(BB->getContext(),
                                                     "",
                                                     BB->getParent());
            assert(splitBB && "Failed to split basic block");
            Instruction *term = BB->getTerminator();

            // Insert a 'check': (This will be expanded by a subsequent
            // iteration of the while loop.)
            IRBuilder<> builder(term);
            Value *check =
              Intrinsic::getDeclaration(M,
                                        Intrinsic::an_check,
                                        x->getType());
            Value *res = builder.CreateCall2(check, x, a);
            Value *zero = ConstantInt::getSigned(res->getType(), 0);
            Value *cmp = builder.CreateICmpNE(res, zero);
            // If the 'check' fails (i.e. returns a non-zero value),
            // branch to the trapping block:
            builder.CreateCondBr(cmp, trapBB, splitBB);
            // The original terminator (inserted by the 'splitBasicBlock'
            // method) is no longer needed):
            term->eraseFromParent();

            IRBuilder<> trapBuilder(trapBB);
            if (ID == Intrinsic::an_assert) {
              // Insert a call to the 'trap' intrinsic: (TODO: Perhaps
              // calling 'exit' with an error value would be more
              // appropriate.)
              Value *trap =
                Intrinsic::getDeclaration(M, Intrinsic::trap);

              trapBuilder.CreateCall(trap);
              trapBuilder.CreateUnreachable();
            } else {
              // Insert a call to 'puts', which prints an error message
              // to the console, but otherwise allows execution to continue
              // at 'splitBB':
              Type *int32Ty   = Type::getInt32Ty(ctx);
              Type *int8PtrTy = Type::getInt8PtrTy(ctx);

              Value *str = M->getGlobalVariable(".str1", true);
              str = trapBuilder.CreateBitCast(str, int8PtrTy);
              FunctionType *fty = FunctionType::get(int32Ty,
                                                    int8PtrTy,
                                                    false);
              Value *puts = M->getOrInsertFunction("puts", fty);
              trapBuilder.CreateCall(puts, str);
              trapBuilder.CreateBr(splitBB);
            }
            // Make the next loop interation visit the newly inserted
            // terminator of 'BB', which is the conditional branch to
            // either 'trapBB' or śplitBB':(In principle the new terminator
            // does not need to be visited since we know that it is not a
            // call to an intrinsic. However, doing the following
            // assignment keeps the logic of the nested while loops
            // consistent).
            N = BB->getTerminator();
            ci->eraseFromParent();
            m = modified = true;
            break;
          }
          case Intrinsic::an_placeholder: {
            // TODO: This is experimental -- only here as "proof of
            // principle".
            assert(ci->getNumUses() == 0 &&
                   "Unhandled placeholdes has uses");
            ci->dump();

            if (ci->getNumArgOperands() == 3 &&
                ci->getArgOperand(0)->getType()->isIntegerTy()) {
              ConstantInt *Arg0 =
                dyn_cast<ConstantInt>(ci->getArgOperand(0));
              if (Arg0) {
                uint64_t i = Arg0->getLimitedValue();
                switch(i) {
                default: break;
                case 0:
                case 1: {
                  IRBuilder<> builder(ci);
                  Intrinsic::ID id = (i == 0) ? Intrinsic::an_assert
                                              : Intrinsic::an_signal;
                  SmallVector<Type*, 2> args;
                  args.push_back(ci->getArgOperand(1)->getType());
                  args.push_back(ci->getArgOperand(1)->getType());
                  Value *intr = Intrinsic::getDeclaration(M, id, args);
                  Value *Arg2 = builder.CreateSExt(ci->getArgOperand(2),
                                                   ci->getArgOperand(1)->getType());
                  builder.CreateCall2(intr,
                                      ci->getArgOperand(1),
                                      Arg2);
                  break;
                }
                }
              }
            }
            ci->eraseFromParent();
            m = modified = true;
            break;
          }
          }
        }
        // Update the iterator for the inner while loop over instructions
        // in a basic block:
        I = N;
      } /* while( I != E) */
      // Update the iterator for the while loop over basic blocks in
      // function 'F':
      BB = BN;
    } /* while( BB != BE) */
  } /* while (m) */

  return modified;
}

static RegisterPass<OperationsExpander> X("OperationsExpander",
                                          "",
                                          false,
                                          false);

Pass *createOperationsExpander() {
  return new OperationsExpander();
}
