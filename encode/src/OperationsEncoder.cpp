
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"

#include "Coder.h"
#include "UsesVault.h"

using namespace llvm;

namespace {
  struct OperationsEncoder : public BasicBlockPass {
    OperationsEncoder(Coder *c) : BasicBlockPass(ID), C(c),
      toggle_before(0), toggle_after(0) {}

    bool runOnBasicBlock(BasicBlock &BB) override;

    static char ID;

    void insertCheckBefore(Value *V, const BasicBlock::iterator &I) {
      // Do not check constants:
      if (dyn_cast<ConstantInt>(V)) return;
#ifdef TOGGLE_CHECKS
      if ((toggle_before++) & 1) return;
#endif
#ifdef CHECK_BEFORE
      C->createAssert(V, &(*I));
#endif
#ifdef ACCU_ARGS
      C->createAccumulate(V, &(*I));
#endif
    }

    void insertCheckAfter(Value *V, const BasicBlock::iterator &I) {
      // Do not check constants:
      if (dyn_cast<ConstantInt>(V)) return;
#ifdef TOGGLE_CHECKS
      if ((toggle_after++) & 1) return;
#endif
#ifdef CHECK_AFTER
      C->createAssert(V, &(*std::next(I)));
#endif
#ifdef ACCU_RES
      C->createAccumulate(V, &(*std::next(I)));
#endif
    }
  private:
    Coder *C;
    unsigned toggle_before, toggle_after;
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
    case Instruction::Load: {
      insertCheckAfter(I, I);
      break;
    }
    case Instruction::Store: {
      insertCheckBefore(I->getOperand(0), I);
      C->createAssert(I->getOperand(0), &(*I));
      break; 
    }
    case Instruction::ICmp: { 
      ICmpInst *cmp = dyn_cast<ICmpInst>(&(*I));
      assert(cmp);
      if (!cmp->isIntPredicate()) break;

      insertCheckBefore(I->getOperand(0), I);
      insertCheckBefore(I->getOperand(1), I);
      break;
    }
    case Instruction::Call: {
      // Deferred to the 'CallHandler' pass.
      break;
    }
    case Instruction::GetElementPtr: {
      // Deferred to the 'GEPHandler' pass.
      break;
    }
    case Instruction::PtrToInt: {
      // Save all current uses of the 'ptrtoint' instruction to the
      // vault. These are the uses that need to be replaced with the
      // encoded integer value: (The 'createEncRegionEntry' method
      // will generate another use of the 'ptrtoint' instruction's
      // result, which msut not be replaced.)
      UsesVault UV(I->uses());

      Value *EncInt = C->createEncRegionEntry(I, std::next(I));

      UV.replaceWith(EncInt);
      modified = true;
      break;
    }
    case Instruction::IntToPtr: {
      Value *Op = I->getOperand(0);
      Value *DecInt = C->createEncRegionExit(Op, Op->getType(), I);
      I->setOperand(0, DecInt);
      modified = true;
      break;
    }
    case Instruction::SDiv:
    case Instruction::UDiv: {
      // FIXME: Handle division instructions properly. (Requires
      // looking into the signed-ness issue.)
      assert(0);
      break;
    }
#define HANDLE_BINOP(OPCODE, NAME)                             \
    case Instruction::OPCODE: {                                \
      Type *int64Ty = Type::getInt64Ty(ctx),                   \
           *origTy = I->getOperand(0)->getType();              \
      \
      Value *ppArg0 = C->preprocessForEncOp(I->getOperand(0), I);  \
      Value *ppArg1 = C->preprocessForEncOp(I->getOperand(1), I);  \
      \
      insertCheckBefore(ppArg0, I);                   \
      insertCheckBefore(ppArg1, I);                   \
      \
      SmallVector<Value*, 2> args;                             \
      args.push_back(ppArg0); \
      args.push_back(ppArg1); \
      \
      Value *Res = C->createEncBinop((NAME), args, I);         \
      \
      insertCheckAfter(Res, I);                      \
      \
      Res = C->postprocessFromEncOp(Res, origTy, I);           \
      I->replaceAllUsesWith(Res);                              \
      I->eraseFromParent();                                    \
      modified = true;                                         \
      break;                                                   \
    }
    HANDLE_BINOP(Add,  "add_enc")
    HANDLE_BINOP(Sub,  "sub_enc")
    HANDLE_BINOP(Mul,  "mul_enc")
    HANDLE_BINOP(URem, "umod_enc")
    HANDLE_BINOP(SRem, "smod_enc")
    HANDLE_BINOP(And,  "and_enc")
    HANDLE_BINOP(Or,   "or_enc")
    HANDLE_BINOP(Shl,  "shl_enc")
    HANDLE_BINOP(LShr, "shr_enc")
    HANDLE_BINOP(AShr, "ashr_enc")
    HANDLE_BINOP(Xor,  "xor_enc")
    }
    I = N;
  }

  return true;
}

Pass *createOperationsEncoder(Coder *c) {
  return new OperationsEncoder(c);
}
