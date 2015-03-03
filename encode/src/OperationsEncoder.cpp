
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
  struct OperationsEncoder : public BasicBlockPass {
    OperationsEncoder() : BasicBlockPass(ID) {}

    bool runOnBasicBlock(BasicBlock &BB) override;

    static char ID;
  };
}

char OperationsEncoder::ID = 0;

bool OperationsEncoder::runOnBasicBlock(BasicBlock &BB) {
  bool modified = false;
  LLVMContext &ctx = BB.getContext();
  Module *M = BB.getParent()->getParent();

  auto I = BB.begin(), E = BB.end();
  while( I != E) {
    auto N = std::next(I);
    unsigned Op = I->getOpcode();
    switch (Op) {
    default: break;
#define HANDLE_BINOP(OPCODE, NAME)    \
    case Instruction::OPCODE: {       \
      Type *int64Ty = Type::getInt64Ty(ctx); \
      Value *Args[2], *Ops[2]; \
      Args[0] = I->getOperand(0); \
      Args[1] = I->getOperand(1); \
      Type *Ty0 = I->getOperand(0)->getType(); \
      Type *Ty1 = I->getOperand(1)->getType(); \
      assert(Ty0->isIntegerTy() && Ty1->isIntegerTy()); \
      unsigned w[2]; \
      w[0] = Ty0->getIntegerBitWidth(); \
      w[1] = Ty1->getIntegerBitWidth(); \
      for (unsigned i = 0; i < 2; i++) { \
        if (w[i] < 64) \
          Ops[i] = new SExtInst(Args[i], int64Ty, "", &(*I)); \
        else if (w[i] > 64) \
          Ops[i] = new TruncInst(Args[i], int64Ty, "", &(*I));\
        else \
          Ops[i] = Args[i]; \
      } \
      SmallVector<Type*, 2> formalArgsTy;     \
      formalArgsTy.push_back(int64Ty); \
      formalArgsTy.push_back(int64Ty); \
      FunctionType *FunTy = FunctionType::get(int64Ty,      \
                                              formalArgsTy, \
                                              false);       \
      Value *F = M->getOrInsertFunction((NAME), FunTy);     \
                                                            \
      SmallVector<Value*, 2> actualArgs;  \
      actualArgs.push_back(Ops[0]);          \
      actualArgs.push_back(Ops[1]);          \
                                          \
      Value *Res = CallInst::Create(F, actualArgs, "", &(*I));  \
      unsigned wi = I->getType()->getIntegerBitWidth(); \
      if ( wi < 64) \
        Res = new TruncInst(Res, I->getType(), "", &(*I));        \
      else if ( wi > 64) \
        Res = new SExtInst(Res, I->getType(), "", &(*I));        \
      I->replaceAllUsesWith(Res);                               \
      I->eraseFromParent();                                     \
      break;                                                    \
    }
    HANDLE_BINOP(Add, "add_enc")
    HANDLE_BINOP(Mul, "mul_enc")
    }
    I = N;
  }

  return true;
}

static RegisterPass<OperationsEncoder> X("OperationsEncoder",
                                         "",
                                         false,
                                         false);

Pass *createOperationsEncoder() {
  return new OperationsEncoder();
}
