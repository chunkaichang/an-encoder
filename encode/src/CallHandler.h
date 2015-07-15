#ifndef __CALL_HANDLER_H__
#define __CALL_HANDLER_H__

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


class ProfiledCoder;

struct CallHandler : public BasicBlockPass {
	CallHandler(ProfiledCoder *pc);

	bool runOnBasicBlock(BasicBlock &BB) override;

	bool handleCallInst(BasicBlock::iterator &I);

	static char ID;

private:
	ProfiledCoder *PC;
	Function *add_with_overflow;
};

Pass *createCallHandler(ProfiledCoder *pc);

#endif /* __CALL_HANDLER_H__ */
