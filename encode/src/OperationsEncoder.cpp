
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"

#include "Coder.h"
#include "ProfiledCoder.h"
#include "UsesVault.h"

using namespace llvm;

namespace {
  struct OperationsEncoder : public BasicBlockPass {
    OperationsEncoder(ProfiledCoder *pc)
    : BasicBlockPass(ID), PC(pc) {}

    bool runOnBasicBlock(BasicBlock &BB) override;

    static char ID;

private:
    ProfiledCoder *PC;
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
    default: {
    	break;
    }
    case Instruction::Alloca:
    case Instruction::Load:
    case Instruction::Store: {
    	modified |= PC->handleMemory(I);
    	break;
    }
    case Instruction::ICmp: { 
    	modified |= PC->handleComparison(I);
    	break;
    }
    case Instruction::Call: {
      modified |= PC->handleCall(I);
      break;
    }
    case Instruction::GetElementPtr: {
    	modified |= PC->handleGEP(I);
      break;
    }
    case Instruction::PtrToInt:
    case Instruction::IntToPtr: {
    	modified |= PC->handlePtrCast(I);
    	break;
    }
    case Instruction::SExt:
    case Instruction::ZExt: {
    	modified |= PC->handleExt(I);
    	break;
    }
    case Instruction::Trunc: {
        modified |= PC->handleTrunc(I);
        break;
    }
#define HANDLE_ARITHMETIC(OPCODE)          \
    case Instruction::OPCODE: {            \
      modified |= PC->handleArithmetic(I); \
      break;                               \
    }
    HANDLE_ARITHMETIC(Add)
    HANDLE_ARITHMETIC(Sub)
    HANDLE_ARITHMETIC(Mul)
    HANDLE_ARITHMETIC(URem)
    HANDLE_ARITHMETIC(SRem)
	HANDLE_ARITHMETIC(UDiv)
	HANDLE_ARITHMETIC(SDiv)

#define HANDLE_BITWISE(OPCODE)           \
    case Instruction::OPCODE: {          \
      modified |= PC->handleBitwise(I);  \
      break;                             \
    }
    HANDLE_BITWISE(And)
    HANDLE_BITWISE(Or)
    HANDLE_BITWISE(Shl)
    HANDLE_BITWISE(LShr)
    HANDLE_BITWISE(AShr)
    HANDLE_BITWISE(Xor)
    }
    I = N;
  }

  return modified;
}

Pass *createOperationsEncoder(ProfiledCoder *pc) {
  return new OperationsEncoder(pc);
}
